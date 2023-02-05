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

resnet = torchvision.models.resnet18(weights=models.ResNet18_Weights.DEFAULT) # pretrained=True)
resnet.eval()
# An example input you would normally provide to your model's forward() method.
example = torch.rand(1, 3, 224, 224)
traced_script_module = torch.jit.trace(resnet, example)
traced_script_module.save("resnet-pretrained-model.pt")

sys.exit(0)
############################################################################
# NOT USED############################################################

# Create the image transform function
transform = transforms.Compose([            #[1]
 transforms.Resize(256),                    #[2]
 transforms.CenterCrop(224),                #[3]
 transforms.ToTensor(),                     #[4]
 transforms.Normalize(                      #[5]
 mean=[0.485, 0.456, 0.406],                #[6]
 std=[0.229, 0.224, 0.225]                  #[7]
 )])

labels=[]
with open('imagenet_classes.txt') as f:
    labels = [line.strip() for line in f.readlines()]

print("Loading image")
img = Image.open("strawberry.jpg")
# Preprocess image

img_t = transform(img)
batch_t = torch.unsqueeze(img_t, 0)


# Apply the ML model
out = resnet(batch_t)

_, index = torch.max(out, 1)
percentage = torch.nn.functional.softmax(out, dim=1)[0] * 100
print(labels[index[0]], percentage[index[0]].item())

# Print the probability of the object
_, indices = torch.sort(out, descending=True)

for idx in indices[0][:5]:
    print(labels[idx],percentage[idx])
#[(labels[idx], percentage[idx].item()) for idx in indices[0][:5]]
#print("END")
