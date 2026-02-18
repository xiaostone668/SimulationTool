const fs = require('fs');
const path = require('path');

const releaseDir = 'd:\\MyAICADCode\\SimulationTool\\build_cmake\\Release';
const searchDirs = [
    releaseDir,
    'C:\\Windows\\System32',
    'C:\\Windows\\SysWOW64',
    'C:\\Windows',
    'D:\\ProgramFiles\\Qt\\6.7.3\\msvc2022_64\\bin'
];

function getDllImports(filePath) {
    try {
        const buf = fs.readFileSync(filePath);
        const peOffset = buf.readUInt32LE(0x3C);
        const sig = buf.toString('ascii', peOffset, peOffset + 2);
        if (sig !== 'PE') return [];
        const numSections = buf.readUInt16LE(peOffset + 6);
        const sizeOfOptHeader = buf.readUInt16LE(peOffset + 20);
        const optHeaderOffset = peOffset + 24;
        const magic = buf.readUInt16LE(optHeaderOffset);
        const is64 = (magic === 0x20B);
        const ddOffset = optHeaderOffset + (is64 ? 0x70 : 0x60);
        const importRva = buf.readUInt32LE(ddOffset + 8);
        if (importRva === 0) return [];
        const sectionTableOffset = optHeaderOffset + sizeOfOptHeader;
        function rvaToOffset(rva) {
            for (let i = 0; i < numSections; i++) {
                const off = sectionTableOffset + i * 40;
                const secVa = buf.readUInt32LE(off + 12);
                const secRawSize = buf.readUInt32LE(off + 16);
                const secRaw = buf.readUInt32LE(off + 20);
                if (rva >= secVa && rva < secVa + secRawSize) return secRaw + (rva - secVa);
            }
            return -1;
        }
        const importOffset = rvaToOffset(importRva);
        if (importOffset < 0) return [];
        const dlls = [];
        let idtOffset = importOffset;
        for (let iter = 0; iter < 500; iter++) {
            const nameRva = buf.readUInt32LE(idtOffset + 12);
            if (nameRva === 0) break;
            const nameOff = rvaToOffset(nameRva);
            if (nameOff < 0) { idtOffset += 20; continue; }
            let end = nameOff;
            while (end < buf.length && buf[end] !== 0) end++;
            const dllName = buf.toString('ascii', nameOff, end);
            if (dllName && /\.dll$/i.test(dllName)) dlls.push(dllName);
            idtOffset += 20;
        }
        return dlls;
    } catch (e) {
        return [];
    }
}

function findDll(dllName) {
    for (const dir of searchDirs) {
        const p = path.join(dir, dllName);
        if (fs.existsSync(p)) return p;
    }
    return null;
}

const visited = new Set();
const missing = [];
const queue = [path.join(releaseDir, 'SimulationTool.exe')];

// Only analyze key DLLs from release dir, skip system DLLs
const systemDllPattern = /^(kernel32|ntdll|user32|gdi32|advapi32|ole32|oleaut32|shell32|ws2_32|comctl32|comdlg32|version|winmm|imm32|opengl32|glu32|d3d|dxgi|msvcrt|api-ms|ext-ms|ucrtbase|vcruntime|msvcp|concrt)/i;

function analyze(filePath) {
    const name = path.basename(filePath).toLowerCase();
    if (visited.has(name)) return;
    visited.add(name);

    const imports = getDllImports(filePath);
    for (const dll of imports) {
        const dllLow = dll.toLowerCase();
        const found = findDll(dll);
        if (!found) {
            // Check if it's a known system DLL pattern
            if (!systemDllPattern.test(dll)) {
                missing.push({ requiredBy: path.basename(filePath), dll });
            }
        } else {
            // Only recurse into non-system DLLs
            if (!systemDllPattern.test(dll) && !visited.has(dllLow)) {
                analyze(found);
            }
        }
    }
}

// Start with the exe and key DLLs in release dir
analyze(path.join(releaseDir, 'SimulationTool.exe'));

// Also check the directly imported DLLs
const directDlls = ['Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','TKernel.dll','TKMath.dll','TKOpenGl.dll','TKService.dll','TKV3d.dll'];
for (const dll of directDlls) {
    const p = path.join(releaseDir, dll);
    if (fs.existsSync(p)) analyze(p);
}

if (missing.length === 0) {
    console.log('All DLL dependencies resolved! No missing DLLs found.');
    console.log(`(Checked ${visited.size} modules)`);
} else {
    console.log('MISSING DLLs:');
    for (const m of missing) {
        console.log(`  [MISSING] ${m.dll}  (required by ${m.requiredBy})`);
    }
}
console.log(`\nAnalyzed ${visited.size} modules`);
