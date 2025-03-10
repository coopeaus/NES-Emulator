import argparse
import os

def main():
    parser = argparse.ArgumentParser(description="Build a custom NES ROM.")
    parser.add_argument("output", nargs="?", default="custom.nes", help="Output path for the ROM file")
    args = parser.parse_args()

    header = bytearray(16)
    header[0:4] = b"NES\x1A"
    header[4] = 0x2  
    header[5] = 0x0  
    header[6] = 0x0  
    header[7] = 0x8  
    header[8] = 0x0  
    header[9] = 0x1  
    header[10] = 0x0 
    header[11] = 0x0
    header[12] = 0x0  
    header[13] = 0x0  
    header[14] = 0x0  
    header[15] = 0x0  

    # PRG-ROM: 16KB
    prg_rom = bytearray(16384)
    prg_rom[16380] = 0x00
    prg_rom[16381] = 0x80

    # CHR-ROM: 8KB
    chr_rom = bytearray(8192)

    output_path = os.path.abspath(args.output)
    with open(output_path, "wb") as f:
        f.write(header)
        f.write(prg_rom)
        f.write(chr_rom)

    print(f"ROM built successfully: {output_path}")

if __name__ == "__main__":
    main()
