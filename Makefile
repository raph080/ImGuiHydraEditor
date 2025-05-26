rhel_install_dep:
	sudo dnf install -y libXt-devel

create_venv:
	python3 -m venv build_venv
	. ./build_venv/bin/activate ;\
		pip install -r ./build_venv_pip.txt

build_open_subdiv:
	cd ./vendors/OpenSubdiv/ && \
	cmake -DCMAKE_INSTALL_PREFIX=./install \
		-DCMAKE_PREFIX_PATH=./install \
		-DCMAKE_BUILD_TYPE=Debug \
		-DNO_EXAMPLES=ON \
		-DNO_TUTORIALS=ON \
		-DNO_REGRESSION=ON \
		-DNO_DOC=ON \
		-DNO_OMP=ON \
		-DNO_CUDA=ON \
		-DNO_OPENCL=ON \
		-DNO_DX=ON \
		-DNO_TESTS=ON \
		-DNO_GLEW=ON \
		-DNO_GLFW=ON \
		-DNO_PTEX=ON \
		-DNO_TBB=ON && \
	cmake --build . --config Debug --target install -j 10 

build_usd: create_venv
	. ./build_venv/bin/activate ; \
		cd ./vendors/OpenUSD/ && \
		python \
		 ./build_scripts/build_usd.py \
		 --build=./build \
		 --build-variant=release \
		 --prefer-safety-over-speed \
		 -v \
		 --no-tests \
		 --no-examples \
		 --no-tutorials \
		 --no-docs \
		 --no-python-docs \
		 --no-debug-python \
		 --no-ptex \
		 --no-draco \
		 --no-mayapy-tests \
		 --no-animx-tests \
		 --python \
		 --tools \
		 --usd-imaging \
		 --openvdb \
		 --usdview \
		 --usdview \
		 --no-prman \
		 --openimageio \
		 --opencolorio \
		 --alembic \
		 --hdf5 \
		 --materialx \
		 --embree \
		 ./install

build_app:
	mkdir -p build && \
	cd build && \
	cmake -Dpxr_DIR=./vendors/OpenUSD/install/ -DOpenSubdiv_DIR=./vendors/OpenSubdiv/install/lib64/cmake/OpenSubdiv -DCMAKE_INSTALL_PREFIX=../install .. && \
	make && \
	make install
	
build:
	$(MAKE) create_venv
	$(MAKE) build_usd

clean:
	rm -rf ./build_venv && \
		rm -rf ./build && \
		rm -rf ./install

run:
	LD_LIBRARY_PATH=./vendors/OpenUSD/install/lib:./vendors/OpenUSD/install/lib64:$LD_LIBRARY_PATH ./install/bin/ImGuiHydraEditor
