from PIL import Image
import struct
import os

FRAME_FOLDER = "frames"
OUTPUT_FILE = "frames.dat"
FRAME_WIDTH = 480
FRAME_HEIGHT = 272
FRAME_COUNT = 6572

def pack_frame(image_path):
    img = Image.open(image_path).convert("L") # convert to greyscale
    img = img.resize((FRAME_WIDTH, FRAME_HEIGHT))
    
    pixels = img.load()
    data = bytearray()

    bit_accum = 0
    bit_count = 0
    for y in range(FRAME_HEIGHT):
        for x in range(FRAME_WIDTH):
            pixel = 1 if pixels[x, y] > 128 else 0 #convert pure greyscale image to stricct black and white
            bit_accum = (bit_accum << 1) | pixel 
            bit_count += 1
            if bit_count == 8:
                data.append(bit_accum)
                bit_accum = 0
                bit_count = 0
    if bit_count > 0:
        data.append(bit_accum << (8 - bit_count))

    return data

def main():
    print(f"Packing {FRAME_COUNT} frames from '{FRAME_FOLDER}'...")

    with open(OUTPUT_FILE, "wb") as out:
        for i in range(1, FRAME_COUNT + 1):
            filename = f"frame_{i:05d}.png"
            path = os.path.join(FRAME_FOLDER, filename)
            if not os.path.exists(path):
                print(f"Missing {filename}, skipping.")
                continue

            frame_data = pack_frame(path)
            out.write(frame_data)

            if i % 100 == 0:
                print(f"Processed {i}/{FRAME_COUNT} frames...")

    print(f"Done! Wrote all frames to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()
