from PIL import Image
import os

FRAME_WIDTH = 480
FRAME_HEIGHT = 272
FRAME_SIZE = (FRAME_WIDTH * FRAME_HEIGHT) // 8
INPUT_FILE = "frames.dat"

def unpack_frame(data):
    img = Image.new("1", (FRAME_WIDTH, FRAME_HEIGHT))
    pixels = img.load()

    byte_index = 0
    for y in range(FRAME_HEIGHT):
        for x in range(0, FRAME_WIDTH, 8):
            byte = data[byte_index]
            byte_index += 1
            for bit in range(8):
                value = (byte >> (7 - bit)) & 1
                if x + bit < FRAME_WIDTH:
                    pixels[x + bit, y] = 255 * value
    return img

def main():
    with open(INPUT_FILE, "rb") as f:
        for frame_num in [0, 100, 500, 1000, 2000, 3000, 4000, 5000, 6000]:
            f.seek(frame_num * FRAME_SIZE)
            frame_data = f.read(FRAME_SIZE)
            if not frame_data:
                print(f"Frame {frame_num} not found!")
                continue

            img = unpack_frame(frame_data)
            output_path = f"preview_{frame_num:05d}.png"
            img.save(output_path)
            print(f"Saved {output_path}")

if __name__ == "__main__":
    main()
