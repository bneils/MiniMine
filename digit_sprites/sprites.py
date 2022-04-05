from PIL import Image

for i in range(1, 8 + 1):
	rows = []
	image = Image.open(str(i)+".png").convert("L")
	assert image.width == 8
	for y in range(image.height):
		row = 0
		for x in range(image.width):
			if image.getpixel((x, y)) < 255//2:
				row |= 1 << (7 - x)
		rows.append("0x" + hex(row).upper()[2:].rjust(2, "0"))
	print("{" + ", ".join(rows) + "},")
