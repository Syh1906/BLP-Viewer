"""从本仓库构建、验证并生成带来源清单的独立 Windows x64 编码组件。"""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = (
    'encoder/CMakeLists.txt', 'encoder/blp_encoder.h', 'encoder/blp_encoder.cpp',
    'encoder/package.py', 'encoder/README.md', 'tests/blp_encoder_api_test.c',
    'src/blp/blp_codec.cpp', 'src/blp/blp_codec.h',
    'third_party/stb/stb_image_resize2.h', 'third_party/stb/stb_image_resize2-LICENSE.txt',
    'third_party/turbojpeg/include/turbojpeg.h',
    'third_party/turbojpeg/lib/turbojpeg-static.lib',
    'third_party/turbojpeg/LICENSE.md', 'third_party/turbojpeg/README.ijg', 'LICENSE',
)


def inventory(paths):
    return {name: {'size': path.stat().st_size, 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}
            for name, path in paths}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cmake', type=Path, required=True)
    parser.add_argument('--build', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    cmake, build, out = (p.resolve() for p in (args.cmake, args.build, args.out))
    if out.exists():
        parser.error('输出目录已存在，不覆盖。')
    before = inventory((name, ROOT/name) for name in SOURCES)
    ctest = cmake.with_name('ctest.exe')
    for command in (
        [cmake, '-S', ROOT/'encoder', '-B', build, '-G', 'Visual Studio 17 2022', '-A', 'x64'],
        [cmake, '--build', build, '--config', 'Release'],
        [ctest, '--test-dir', build, '-C', 'Release', '--output-on-failure'],
        [cmake, '--install', build, '--config', 'Release', '--prefix', out],
    ):
        subprocess.run(list(map(str, command)), check=True)
    after = inventory((name, ROOT/name) for name in SOURCES)
    if before != after:
        raise RuntimeError('构建期间来源文件变化；候选不能交付，请在新输出位置重新构建。')
    dll = ctypes.CDLL(str(out/'bin/blp_encoder.dll'))
    dll.blp_encoder_version.restype = ctypes.c_char_p
    dll.blp_encoder_abi_version.restype = ctypes.c_uint32
    source_digest = hashlib.sha256(json.dumps(before, sort_keys=True, separators=(',', ':')).encode()).hexdigest()
    manifest = {
        'schema_version': 1,
        'package_id': 'blp-encoder-1.0.0-' + source_digest[:12] + '-windows-x64',
        'version': dll.blp_encoder_version().decode('utf-8'), 'abi_version': dll.blp_encoder_abi_version(),
        'platform': 'windows-x64', 'configuration': 'Release / MD',
        'source_commit': subprocess.check_output(['git', '-C', str(ROOT), 'rev-parse', 'HEAD'], text=True).strip(),
        'source_dirty': bool(subprocess.check_output(['git', '-C', str(ROOT), 'status', '--porcelain'], text=True)),
        'source_files': before, 'source_fingerprint': source_digest,
        'files': inventory((p.relative_to(out).as_posix(), p) for p in sorted(out.rglob('*')) if p.is_file()),
    }
    (out/'manifest.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print(json.dumps({'package': out.as_posix(), 'package_id': manifest['package_id'],
                      'version': manifest['version'], 'abi_version': manifest['abi_version'],
                      'source_dirty': manifest['source_dirty'], 'files': len(manifest['files'])}, ensure_ascii=False))


if __name__ == '__main__':
    main()
