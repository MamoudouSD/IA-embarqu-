# Diani Mamoudou S
#Decembre  2024
#Python version 3.10.16 - numpy version 1.26.4 - pillow version 11.0.0 - tflite_runtime version 2.14.0


import numpy as np
from PIL import Image
import tflite_runtime.interpreter as tflite
import time


# 1. Chargement et pretraitement de l'image
def preprocess_image(image_path, target_size):
    img = Image.open(image_path).convert("L")
    img = img.resize((target_size,target_size)) 
    img_array = np.array(img).astype(np.float32) 
    img_array = img_array / 255.0  
    img_array = img_array.reshape( target_size,target_size,1)
    return np.expand_dims(img_array, axis=0)  


# 2. Chargement un modèle TensorFlow Lite.
def load_model(model_path):
    interpreter = tflite.Interpreter(model_path=model_path)
    interpreter.allocate_tensors()
    return interpreter


# 3. Exécution de l'inférence sur le modèle avec les données d'entrée.
def run_inference(interpreter, input_data):
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output_data = interpreter.get_tensor(output_details[0]['index'])
    return output_data


# 4. Main
if __name__ == "__main__":
    model_path = "model/model_quantized_and_pruned_tflite.tflite"
    image_path = "annexe/image_capturee.png"
    start = time.time()

    input_data = preprocess_image(image_path, target_size=120)
    interpreter = load_model(model_path)
    result = run_inference(interpreter, input_data)

    pred = (list(result[0]).index(1))+1
    match pred:
        case 1:
            prediction = "Palm"
        case 2:
            prediction = "L"
        case 3:
            prediction = "Fist"
        case 4:
            prediction = "Fist_moved"
        case 5:
            prediction = "Thumb"
        case 6:
            prediction = "Index"
        case 7:
            prediction = "Ok"
        case 8:
            prediction = "Palm_moved"
        case 9:
            prediction = "C"
        case 10:
            prediction = "Down"
    fichier = open("resultat.txt", "a")
    fichier.write(prediction+"\n")
    fichier.close()
    print("Résultat du modèle : ", prediction)