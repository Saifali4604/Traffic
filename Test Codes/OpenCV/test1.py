import cv2
import numpy as np
import urllib.request
import concurrent.futures
import time

url = 'http://192.168.208.35/cam-hi.jpg'

# Load YOLO
net = cv2.dnn.readNet("yolov3.weights", "yolov3.cfg")
layer_names = net.getLayerNames()

# Fetching output layer names depending on OpenCV version
try:
    output_layers = [layer_names[i - 1] for i in net.getUnconnectedOutLayers()]
except TypeError:
    output_layers = [layer_names[i[0] - 1] for i in net.getUnconnectedOutLayers()]

with open("coco.names", "r") as f:
    classes = [line.strip() for line in f.readlines()]

def fetch_image(url, retries=3, timeout=5):
    for i in range(retries):
        try:
            img_resp = urllib.request.urlopen(url, timeout=timeout)
            imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
            im = cv2.imdecode(imgnp, -1)
            return im
        except Exception as e:
            print(f"Attempt {i+1} failed: {e}")
            time.sleep(1)
    return None

def run1():
    cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)
    while True:
        im = fetch_image(url)
        if im is None:
            print("Failed to fetch image. Exiting.")
            break
        cv2.imshow('live transmission', im)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break
    cv2.destroyAllWindows()

def run2():
    cv2.namedWindow("detection", cv2.WINDOW_AUTOSIZE)
    while True:
        im = fetch_image(url)
        if im is None:
            print("Failed to fetch image. Exiting.")
            break

        # Detecting objects
        blob = cv2.dnn.blobFromImage(im, 0.00392, (416, 416), (0, 0, 0), True, crop=False)
        net.setInput(blob)
        outs = net.forward(output_layers)

        # Showing information on the screen
        class_ids = []
        confidences = []
        boxes = []
        for out in outs:
            for detection in out:
                scores = detection[5:]
                class_id = np.argmax(scores)
                confidence = scores[class_id]
                if confidence > 0.5:
                    # Object detected
                    center_x = int(detection[0] * im.shape[1])
                    center_y = int(detection[1] * im.shape[0])
                    w = int(detection[2] * im.shape[1])
                    h = int(detection[3] * im.shape[0])

                    # Rectangle coordinates
                    x = int(center_x - w / 2)
                    y = int(center_y - h / 2)

                    boxes.append([x, y, w, h])
                    confidences.append(float(confidence))
                    class_ids.append(class_id)

        indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.5, 0.4)

        vehicle_count = 0
        for i in range(len(boxes)):
            if i in indexes:
                x, y, w, h = boxes[i]
                label = str(classes[class_ids[i]])
                if label in ['car', 'truck', 'bus', 'motorcycle']:
                    vehicle_count += 1
                    cv2.rectangle(im, (x, y), (x + w, y + h), (0, 255, 0), 2)
                    cv2.putText(im, label, (x, y + 30), cv2.FONT_HERSHEY_PLAIN, 3, (0, 255, 0), 3)

        print(f"Number of vehicles: {vehicle_count}")
        cv2.imshow('detection', im)
        key = cv2.waitKey(5)
        if key == ord('q'):
            break
    cv2.destroyAllWindows()

if _name_ == '_main_':
    print("started")
    with concurrent.futures.ThreadPoolExecutor() as executor:
        f1 = executor.submit(run1)
        f2 = executor.submit(run2)

    # Optionally, wait for both tasks to complete
    concurrent.futures.wait([f1, f2])
