from ultralytics import YOLO
import cv2
import numpy as np


model=YOLO("best.pt")
classNames=['fire','smoke']

video=cv2.VideoCapture("test3.mp4")
while True:
    flag,frame=video.read()
    result=model(frame,stream=True)
    for r in result:
        boxes=r.boxes
        for box in boxes:
            conf=box.conf[0].item()
            if conf<0.2:
                continue
            x1,y1,x2,y2=box.xyxy[0]
            x1,y1,x2,y2=int(x1),int(x2),int(y1),int(y2)
            cv2.rectangle(frame,(x1,y1),(x2,y2),(0,255,0),thickness=2)
            
            classnum=int(box.cls[0])
            classname=classNames[classnum]
            text = f'{classname}: {conf:.2f}'
            cv2.putText(frame, text, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
    cv2.imshow("test",frame)
    if cv2.waitKey(30) & 0xFF==27:
        break

        

