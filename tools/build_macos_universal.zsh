#!/usr/bin/env zsh

set -eu -o pipefail

[[ "$(arch)" == "arm64" ]] && arch="arm64" || arch="x86_64"
basedir=$(dirname "$0")/..
maa_debug=${MAA_DEBUG:-0}
pushd ${basedir}

build_arch() {
    // remove previous artifacts
    rm -rf build-$1
    [[ $1 = "arm64" ]] && triplet="arm64-osx" || triplet="x64-osx"

    python3 maadeps-download.py ${triplet}

    if [[ -n $(which ccache) ]]; then
        export CMAKE_C_COMPILER_LAUNCHER=ccache
        export CMAKE_CXX_COMPILER_LAUNCHER=ccache
    fi

    cmake -B build-$1 -GNinja -DCMAKE_BUILD_TYPE=$2 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_OSX_ARCHITECTURES=$1
    cmake --build build-$1
    cmake --install build-$1 --prefix install-$1
}

if [[ ${maa_debug} -eq 1 ]]; then
    build_arch "${arch}" "Debug"
elif [[ ${maa_debug} -eq 0 ]]; then
    build_arch "arm64" "Release"
    build_arch "x86_64" "Release"
else
    echo "Invalid MAA_DEBUG value: ${maa_debug}, please set to 0 or 1."
    exit 1
fi

rm -rf build
mkdir build
for LIB_NAME in $(ls install-"${arch}" | grep .dylib); do
    if [[ "${maa_debug}" -eq 1 ]]; then
        mv install-"${arch}"/$LIB_NAME build/$LIB_NAME
    else
        lipo -create install-arm64/$LIB_NAME install-x86_64/$LIB_NAME -output build/$LIB_NAME
    fi
done
cp build-"${arch}"/compile_commands.json build

pushd build
xcodebuild -create-xcframework -library libMaaCore.dylib -headers ../include -output MaaCore.xcframework
xcodebuild -create-xcframework -library libfastdeploy_ppocr.dylib -output fastdeploy_ppocr.xcframework
xcodebuild -create-xcframework -library libonnxruntime.*.dylib -output ONNXRuntime.xcframework
xcodebuild -create-xcframework -library libopencv*.dylib -output OpenCV.xcframework
rm -rf *.dylib
popd

rm -rf install-arm64 install-x86_64
popd
