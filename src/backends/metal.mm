// backend.mm
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"
#include "backend.h"

static NSWindow* window = nil;
static NSView* contentView = nil;
static CAMetalLayer* metalLayer = nil;
static id<MTLDevice> device = nil;
static id<MTLCommandQueue> commandQueue = nil;
static bool shouldClose = false;
static bool backendReady = false;

static id<CAMetalDrawable> currentDrawable = nil;
static MTLRenderPassDescriptor* currentRenderPassDescriptor = nil;

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    NSRect frame = NSMakeRect(0, 0, 1280, 720);
    window = [[NSWindow alloc] initWithContentRect:frame
                                          styleMask:(NSWindowStyleMaskTitled |
                                                     NSWindowStyleMaskClosable |
                                                     NSWindowStyleMaskResizable)
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    [window setTitle:@"ImGui Metal App"];
    [window setDelegate:self];
    [window makeKeyAndOrderFront:nil];

    device = MTLCreateSystemDefaultDevice();
    commandQueue = [device newCommandQueue];

    contentView = [[NSView alloc] initWithFrame:frame];
    [window setContentView:contentView];

    metalLayer = [CAMetalLayer layer];
    metalLayer.device = device;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.framebufferOnly = YES;
    metalLayer.contentsScale = window.backingScaleFactor;
    metalLayer.frame = contentView.bounds;
    [contentView setLayer:metalLayer];
    [contentView setWantsLayer:YES];

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplMetal_Init(device);
    ImGui_ImplOSX_Init(contentView);

    backendReady = true;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
    shouldClose = true;
    [metalLayer removeFromSuperlayer];
    metalLayer = nil;
    contentView = nil;
    window = nil;
}
@end

void InitBackend() {
    [NSApplication sharedApplication];
    static AppDelegate* delegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:delegate];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    // Wait until AppDelegate finishes initialization
    while (!backendReady) {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantPast]];
    }
}

bool ShouldCloseApp() {
    return shouldClose;
}

void PollEvents() {
    NSEvent* event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:nil
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
        [NSApp sendEvent:event];
    }
}

bool BeginFrame() {
    if (shouldClose) return false;
    currentDrawable = [metalLayer nextDrawable];
    if (!currentDrawable) return false;

    currentRenderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    currentRenderPassDescriptor.colorAttachments[0].texture = currentDrawable.texture;
    currentRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    currentRenderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    currentRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.45, 0.55, 0.60, 1.0);

    ImGui_ImplMetal_NewFrame(currentRenderPassDescriptor);
    ImGui_ImplOSX_NewFrame(contentView);

    return true;
}

void EndFrame() {
    if (shouldClose || !currentDrawable || !currentRenderPassDescriptor) return;

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:currentRenderPassDescriptor];

    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, encoder);

    [encoder endEncoding];
    [commandBuffer presentDrawable:currentDrawable];
    [commandBuffer commit];

    currentDrawable = nil;
    currentRenderPassDescriptor = nil;
}

void ShutdownBackend() {
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();
}

void *GetPointerToHgiTextureBackend(pxr::HgiTextureHandle texHandle, float width, float height)
{
    id<MTLTexture> mtlTex = (id<MTLTexture>) texHandle->GetRawResource();

    ImVec2 size = ImVec2(width, height); // Set AOV resolution
    ImTextureID textureID = (__bridge void*)mtlTex;
    return (void *)textureID;
}