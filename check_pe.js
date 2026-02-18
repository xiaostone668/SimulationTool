const fs = require('fs');
const path = require('path');

const exePath = 'd:\\MyAICADCode\\SimulationTool\\build_cmake\\Release\\SimulationTool.exe';
const releaseDir = 'd:\\MyAICADCode\\SimulationTool\\build_cmake\\Release';

const buf = fs.readFileSync(exePath);

// Read PE offset at 0x3C
const peOffset = buf.readUInt32LE(0x3C);
const sig = buf.toString('ascii', peOffset, peOffset + 4);
if (sig.slice(0,2) !== 'PE') { console.log('Not PE'); process.exit(1); }

const machine = buf.readUInt16LE(peOffset + 4);
const numSections = buf.readUInt16LE(peOffset + 6);
const sizeOfOptHeader = buf.readUInt16LE(peOffset + 20);
const optHeaderOffset = peOffset + 24;
const magic = buf.readUInt16LE(optHeaderOffset);
const is64 = (magic === 0x20B);
console.log(`Machine: ${machine.toString(16)}, is64: ${is64}`);

// Import directory is DataDirectory[1]
// For PE32+: DataDirectory starts at offset 0x70 from optHeader
// For PE32:  DataDirectory starts at offset 0x60 from optHeader
const ddOffset = optHeaderOffset + (is64 ? 0x70 : 0x60);
const importRva = buf.readUInt32LE(ddOffset + 8);
const importSize = buf.readUInt32LE(ddOffset + 12);
console.log(`Import RVA: 0x${importRva.toString(16)}, Size: ${importSize}`);

// Section table
const sectionTableOffset = optHeaderOffset + sizeOfOptHeader;

function rvaToOffset(rva) {
    for (let i = 0; i < numSections; i++) {
        const off = sectionTableOffset + i * 40;
        const secVa = buf.readUInt32LE(off + 12);
        const secRawSize = buf.readUInt32LE(off + 16);
        const secRaw = buf.readUInt32LE(off + 20);
        if (rva >= secVa && rva < secVa + secRawSize) {
            return secRaw + (rva - secVa);
        }
    }
    return -1;
}

const importOffset = rvaToOffset(importRva);
console.log(`Import file offset: 0x${importOffset.toString(16)}`);

const dlls = [];
let idtOffset = importOffset;
while (true) {
    // Import Directory Table: 20 bytes per entry
    const nameRva = buf.readUInt32LE(idtOffset + 12);
    if (nameRva === 0) break;
    const nameOff = rvaToOffset(nameRva);
    if (nameOff < 0) { idtOffset += 20; continue; }
    let end = nameOff;
    while (buf[end] !== 0) end++;
    const dllName = buf.toString('ascii', nameOff, end);
    dlls.push(dllName);
    idtOffset += 20;
}

console.log('\nImported DLLs:');
console.log('==============');

const sysDir = 'C:\\Windows\\System32';
const sysDir32 = 'C:\\Windows\\SysWOW64';

for (const dll of dlls.sort()) {
    const inRelease = fs.existsSync(path.join(releaseDir, dll));
    const inSys = fs.existsSync(path.join(sysDir, dll));
    const inSys32 = fs.existsSync(path.join(sysDir32, dll));
    if (inRelease) {
        console.log(`  [OK]      ${dll}  (Release dir)`);
    } else if (inSys) {
        console.log(`  [OK]      ${dll}  (System32)`);
    } else if (inSys32) {
        console.log(`  [OK]      ${dll}  (SysWOW64)`);
    } else {
        console.log(`  [MISSING] ${dll}  <-- NOT FOUND!`);
    }
}
