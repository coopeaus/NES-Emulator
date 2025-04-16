import unittest
import emu


class TestExample(unittest.TestCase):
    def test_bug_1(self):
        e = emu.Emulator()
        e.load("../../roms/palette.nes")
        e.debug_reset()
        pal0 = e.ppu_read(0x3F00)
        steps = 0
        while pal0 == e.ppu_read(0x3F00):
            e.step()
            steps += 1
        print(f"Steps: {steps}")
        e.log()


if __name__ == "__main__":
    unittest.main()
