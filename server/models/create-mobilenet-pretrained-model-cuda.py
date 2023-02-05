# combined example using alexnet and resnet18
# https://learnopencv.com/pytorch-for-beginners-image-classification-using-pre-trained-models/

# Import Pillow

from PIL import Image
import sys
import torch
import torchvision
from torchvision import models
from torchvision import transforms

print("Loading model")
# An instance of your model.

resnet = torchvision.models.mobilenet_v3_large(pretrained=True)
resnet.to('cuda') ## Attempt to translate model to be cuda optimized
resnet.eval()
# An example input you would normally provide to your model's forward() method.
example = torch.rand(1, 3, 224, 224).to('cuda')
traced_script_module = torch.jit.trace(resnet, example)
traced_script_module.save("mobilenet-pretrained-model.pt")

sys.exit(0)
