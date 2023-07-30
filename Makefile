.PHONY: build
build:
	cmake -B build -S . \
	-DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
	-DCMAKE_BUILD_TYPE=Debug 
	cmake --build build

.PHONY: run-client
run-client: build
	./build/client $(name)

.PHONY: run-server
run-server: build
	./build/server 

.PHONY: fmt
fmt:
	clang-format -i common/**.h common/**.cpp server/**.h server/**.cpp client/**.h client/**.cpp  
