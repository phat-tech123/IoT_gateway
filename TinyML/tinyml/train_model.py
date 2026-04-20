import pandas as pd
import numpy as np
import tensorflow as tf
import joblib

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, confusion_matrix

PREFIX = "TinyML"

# =========================
# 1️⃣ LOAD DATA (auto detect separator)
# =========================
data = pd.read_csv("data_sensor.csv", sep=None, engine="python")

print("Dataset shape:", data.shape)
print(data.head())

# Nếu file không có header
if data.shape[1] == 3:
    data.columns = ["temp", "humidity", "label"]

X = data[["temp", "humidity"]].values
y = data["label"].values

# =========================
# 2️⃣ CHECK CLASS DISTRIBUTION
# =========================
print("\nClass distribution:")
print(pd.Series(y).value_counts())

# =========================
# 3️⃣ TRAIN / TEST SPLIT (TRƯỚC KHI SCALE)
# =========================
X_train, X_test, y_train, y_test = train_test_split(
    X, y,
    test_size=0.2,
    random_state=42,
    stratify=y   # giữ cân bằng class
)

# =========================
# 4️⃣ NORMALIZE ĐÚNG CÁCH
# =========================
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

print("Mean:", scaler.mean_)
print("Std:", scaler.scale_)

joblib.dump(scaler, "scaler.save")

# =========================
# 5️⃣ BUILD MODEL (ỔN ĐỊNH HƠN)
# =========================
model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(2,)),
    tf.keras.layers.Dense(16, activation='relu'),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(3, activation='softmax')
])

model.compile(
    optimizer='adam',
    loss='sparse_categorical_crossentropy',
    metrics=['accuracy']
)

# =========================
# 6️⃣ TRAIN
# =========================
history = model.fit(
    X_train,
    y_train,
    epochs=80,
    validation_data=(X_test, y_test),
    verbose=1
)

# =========================
# 7️⃣ EVALUATE
# =========================
loss, acc = model.evaluate(X_test, y_test)
print(f"\nTest Accuracy: {acc:.4f}")

y_pred = np.argmax(model.predict(X_test), axis=1)

print("\nConfusion Matrix:")
print(confusion_matrix(y_test, y_pred))

print("\nClassification Report:")
print(classification_report(y_test, y_pred))

# =========================
# 8️⃣ SAVE MODEL
# =========================
model.save(PREFIX + ".h5")

# =========================
# 9️⃣ CONVERT TO TFLITE (Quantized)
# =========================
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open(PREFIX + ".tflite", "wb") as f:
    f.write(tflite_model)

print("TFLite size:", len(tflite_model), "bytes")

# =========================
# 🔟 EXPORT TO C HEADER
# =========================
with open(PREFIX + ".tflite", "rb") as f:
    tflite_content = f.read()

hex_array = ',\n  '.join(
    ', '.join(f'0x{byte:02x}' for byte in tflite_content[i:i+12])
    for i in range(0, len(tflite_content), 12)
)

with open(PREFIX + ".h", "w") as header_file:
    header_file.write('const unsigned char model[] = {\n  ')
    header_file.write(hex_array)
    header_file.write('\n};\n')
    header_file.write(f'const unsigned int model_len = {len(tflite_content)};\n')

print("Header file generated:", PREFIX + ".h")