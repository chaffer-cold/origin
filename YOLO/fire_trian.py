
from ultralytics import YOLO

'''
print(torch.__version__)
gpuAvailable=torch.cuda.is_available()
print(gpuAvailable)
gpu=torch.cuda.get_device_name()
print(gpu)
print(torch.version.cuda)
'''
def main():
    model=YOLO('best.pt')
    #model.to('cpu') 

    model.train(data='G:/project/yolo/synthetic_fire_smoke.v17i.yolov8/mydata.yaml',epochs=100)

    model.val()
if __name__ == '__main__': # 不加这句就会报错
    main()