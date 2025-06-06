#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"
#include "backend.h"

#include <pxr/imaging/hdx/hgiConversions.h>
#include <pxr/imaging/hgi/blitCmdsOps.h>
#include <iostream>

static void (*AppFrameCallback)() = nullptr;
static pxr::HgiTextureHandle hgiTexture;
static const char* appTitle;
static int appWidth;
static int appHeight;

@interface AppViewController : NSViewController<NSWindowDelegate>
@end


@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

@implementation AppViewController

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }

    // Setup Renderer backend
    ImGui_ImplMetal_Init(_device);

    return self;
}

-(MTKView *)mtkView
{
    return (MTKView *)self.view;
}

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, appWidth, appHeight)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    self.mtkView.device = self.device;
    self.mtkView.delegate = self;

    ImGui_ImplOSX_Init(self.view);
    [NSApp activateIgnoringOtherApps:YES];
}

-(void)drawInMTKView:(MTKView*)view
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;


    CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;

    io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);

    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor == nil)
    {
        [commandBuffer commit];
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
    ImGui_ImplOSX_NewFrame(view);

    AppFrameCallback();

    ImDrawData* draw_data = ImGui::GetDrawData();

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
    ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

    // Present
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    self.view.window.delegate = self;
}

- (void)windowWillClose:(NSNotification *)notification
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();
}

@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@end

@implementation AppDelegate

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(instancetype)init
{
    if (self = [super init])
    {
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self.window.title = [NSString stringWithUTF8String:appTitle];
        self.window.contentViewController = rootViewController;
        [self.window center];
        [self.window makeKeyAndOrderFront:self];
    }
    return self;
}

@end

int InitBackend(const char* title, int width, int height)
{
    appTitle = title;
    appWidth = width;
    appHeight = height;

    @autoreleasepool
    {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        AppDelegate *appDelegate = [[AppDelegate alloc] init];   // creates window
        [NSApp setDelegate:appDelegate];

        [NSApp activateIgnoringOtherApps:YES];
        
    }

    return 0;
}

void RunBackend(void (*callback)())
{
    AppFrameCallback = callback;

    @autoreleasepool
    {
        [NSApp run];
    }
}

void ShutdownBackend()
{ 
}

void *GetPointerToTextureBackend(pxr::HdRenderBuffer* buffer, pxr::Hgi* hgi)
{
    if (!buffer || !buffer->IsConverged()) {
        std::cerr << "Render buffer not ready." << std::endl;
        return nullptr;
    }

    void* pixelData = buffer->Map();
    int width = buffer->GetWidth();
    int height = buffer->GetHeight();
    int depth = buffer->GetDepth();
    pxr::HdFormat hdFormat = buffer->GetFormat();

    const pxr::GfVec3i dim( width, height, depth);

    const pxr::HgiFormat bufFormat = pxr::HdxHgiConversions::GetHgiFormat(hdFormat);
    const size_t pixelByteSize = pxr::HdDataSizeOfFormat(hdFormat);
    const size_t dataByteSize = dim[0] * dim[1] * dim[2] * pixelByteSize;

    // Update the existing texture if specs are compatible. This is more
    // efficient than re-creating, because the underlying framebuffer that
    // had the old texture attached would also need to be re-created.
    if (hgiTexture && hgiTexture->GetDescriptor().dimensions == dim &&
            hgiTexture->GetDescriptor().format == bufFormat) {
        pxr::HgiTextureCpuToGpuOp copyOp;
        copyOp.bufferByteSize = dataByteSize;
        copyOp.cpuSourceBuffer = pixelData;
        copyOp.gpuDestinationTexture = hgiTexture;
        pxr::HgiBlitCmdsUniquePtr blitCmds = hgi->CreateBlitCmds();
        blitCmds->PushDebugGroup("Update ImGui Image Texture");
        blitCmds->CopyTextureCpuToGpu(copyOp);
        blitCmds->PopDebugGroup();
        hgi->SubmitCmds(blitCmds.get());
    } else {
        // Destroy old texture
        if(hgiTexture) {
            hgi->DestroyTexture(&hgiTexture);
        }
        // Create a new texture
        pxr::HgiTextureDesc texDesc;
        texDesc.debugName = "ImGui Image Texture";
        texDesc.dimensions = dim;
        texDesc.format = bufFormat;
        texDesc.initialData = pixelData;
        texDesc.layerCount = 1;
        texDesc.mipLevels = 1;
        texDesc.pixelsByteSize = dataByteSize;
        texDesc.sampleCount = pxr::HgiSampleCount1;
        texDesc.usage = pxr::HgiTextureUsageBitsShaderRead;

        hgiTexture = hgi->CreateTexture(texDesc);
    }

    buffer->Unmap();

    id<MTLTexture> mtlTex = (id<MTLTexture>) hgiTexture->GetRawResource();
    ImTextureID textureID = (__bridge void*)mtlTex;
    return (void *)textureID;
}