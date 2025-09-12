import pybilde
import fargv
from PIL import Image
import numpy as np


def feature_extractor_main():
    p = {
        images: set([]),
        radii: "1,2,3,4,5,6,7,8,9,10,11,12",
    }
    args, _ = fargv.parse(p, "Feature extractor for Bilde")
    images = args.images
    radii = args.radii
    radii = [int(r) for r in radii.split(",")]
    for image in images:
        img = Image.open(image)
        img = img.convert("L")
        
