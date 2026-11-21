#!/usr/bin/env python3
"""Build a show: content/<show>/ -> build/<show>/

    *.svg  -> *.bsm      (svg2bsm.py)
    *.msg  -> *.msg.o    (msgc.py)
    *.ogg  -> copied unchanged

The output mirrors the folder layout of the sources.  The build is incremental
(a file is rebuilt when its source or the converting tool is newer) and the SVG
conversions run in parallel.  Afterwards every model referenced by the scripts
is checked for a matching .bsm, because a missing shape would otherwise only
show up at run time.

    build_content.py [--show masagin] [--root .] [-j N] [--force] [--keep-going]
"""
import argparse
import os
import shutil
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

HERE = os.path.dirname(os.path.abspath(__file__))
COPY_EXT = ('.ogg',)          # files that go into the build unchanged


def newer(src, dst, *deps):
    """True if dst is missing or older than src / any dep."""
    if not os.path.isfile(dst):
        return True
    t = os.path.getmtime(dst)
    return any(os.path.getmtime(p) > t for p in (src,) + deps)


def collect(content_dir, out_dir):
    svgs, msgs, copies = [], [], []
    for dirpath, dirnames, filenames in os.walk(content_dir):
        dirnames[:] = sorted(d for d in dirnames if not d.startswith('.'))
        rel = os.path.relpath(dirpath, content_dir)
        target = out_dir if rel == '.' else os.path.join(out_dir, rel)
        for fn in sorted(filenames):
            if fn.startswith('.'):
                continue
            base, ext = os.path.splitext(fn)
            src = os.path.join(dirpath, fn)
            if ext == '.svg':
                svgs.append((src, os.path.join(target, base + '.bsm')))
            elif ext == '.msg':
                msgs.append((src, os.path.join(target, base + '.msg.o')))
            elif ext in COPY_EXT:
                copies.append((src, os.path.join(target, fn)))
    return svgs, msgs, copies


def run_tool(tool, src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    tmp = dst + '.tmp'
    p = subprocess.run([sys.executable, os.path.join(HERE, tool), src, tmp],
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                       errors='replace')
    if p.returncode != 0 or not os.path.isfile(tmp):
        if os.path.isfile(tmp):
            os.remove(tmp)
        return False, p.stdout
    os.replace(tmp, dst)          # never leave a half-written output behind
    return True, p.stdout


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--show', default='masagin', help='which folder under content/ to build')
    ap.add_argument('--root', default='.', help='project root holding content/ and build/')
    ap.add_argument('-j', '--jobs', type=int, default=os.cpu_count() or 1)
    ap.add_argument('--force', action='store_true', help='rebuild everything')
    ap.add_argument('-k', '--keep-going', action='store_true', help='do not stop at first error')
    a = ap.parse_args()

    content = os.path.abspath(os.path.join(a.root, 'content', a.show))
    out = os.path.abspath(os.path.join(a.root, 'build', a.show))
    if not os.path.isdir(content):
        sys.exit('no such content folder: %s' % content)

    svgs, msgs, copies = collect(content, out)
    svg2bsm = os.path.join(HERE, 'svg2bsm.py')
    msgc = os.path.join(HERE, 'msgc.py')
    svgmesh = os.path.join(HERE, 'svgmesh.py')
    tm = os.path.join(HERE, 'tinymarshal.py')
    failures = []
    t0 = time.time()

    for src, dst in copies:
        if a.force or newer(src, dst):
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copyfile(src, dst)

    def stage(label, items, tool, *deps):
        todo = [(s, d) for s, d in items if a.force or newer(s, d, *deps)]
        print('%s: %d to build, %d up to date' % (label, len(todo), len(items) - len(todo)))
        with ThreadPoolExecutor(max_workers=max(1, a.jobs)) as ex:
            futs = {ex.submit(run_tool, tool, s, d): s for s, d in todo}
            for f in as_completed(futs):
                ok, log = f.result()
                src = os.path.relpath(futs[f], content)
                if ok:
                    print('  ok    %s' % src)
                else:
                    print('  FAIL  %s' % src)
                    print('\n'.join('        ' + l for l in log.strip().splitlines()[-8:]))
                    failures.append(src)
        if failures and not a.keep_going:
            sys.exit('stopped: %d failure(s)' % len(failures))

    # shapes first, scripts second: the scripts reference their shapes
    stage('shapes ', svgs, 'svg2bsm.py', svg2bsm, svgmesh, tm)
    stage('scripts', msgs, 'msgc.py', msgc, tm)

    # sanity: every model referenced by the compiled demo must exist as a .bsm
    demo_src = os.path.join(content, 'demo.msg')
    if os.path.isfile(demo_src):
        sys.path.insert(0, HERE)
        import msgc as msgc_mod
        c = msgc_mod.MSGCompiler()
        c.read(demo_src)
        missing = [m for m in c.models if not os.path.isfile(os.path.join(out, m + '.bsm'))]
        print('demo references %d models, %d missing' % (len(c.models), len(missing)))
        for m in missing:
            print('  missing: %s.bsm' % m)
        if missing:
            failures.append('missing models')

    print('done in %.1fs, %d failure(s)' % (time.time() - t0, len(failures)))
    sys.exit(1 if failures else 0)


if __name__ == '__main__':
    main()
