#encode the sprites into pixel data
from PIL import Image

for i in range(1, 8 + 1):
	rows = []
	image = Image.open(f"_{i}.png").convert("RGB")
	assert image.width == 8
	for y in range(image.height):
		row = 0
		for x in range(image.width):
			if image.getpixel((x, y)) != (246, 135, 207):
				row |= 1 << x
		rows.append("0x" + hex(row).upper()[2:].rjust(2, "0"))
	print("{" + ", ".join(rows) + "},")
