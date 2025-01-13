from flask import Flask, render_template, request, redirect, url_for, flash, jsonify
from flask_sqlalchemy import SQLAlchemy
from flask_login import LoginManager, UserMixin, login_user, login_required, logout_user, current_user
import paho.mqtt.client as mqtt
import json
import os

app = Flask(__name__)
app.secret_key = 'your_secret_key'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///app.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)
login_manager = LoginManager(app)
login_manager.login_view = 'login'

mqtt_client = mqtt.Client()

# Models
class User(UserMixin, db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(150), unique=True, nullable=False)
    password = db.Column(db.String(150), nullable=False)

class Device(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    mac_address = db.Column(db.String(17), unique=True, nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

class SensorData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    device_id = db.Column(db.Integer, db.ForeignKey('device.id'), nullable=False)
    time = db.Column(db.Integer, nullable=False)
    temperature = db.Column(db.Float, nullable=False)
    pressure = db.Column(db.Float, nullable=False)
    moisture = db.Column(db.Float, nullable=False)

# Login manager
@login_manager.user_loader
def load_user(user_id):
    return db.session.get(User, int(user_id))

@app.route('/')
@login_required
def index():
    devices = Device.query.filter_by(user_id=current_user.id).all()
    return render_template('dashboard.html', devices=devices)

@app.route('/register_device', methods=['POST'])
@login_required
def register_device():
    mac_address = request.form['mac_address']
    if Device.query.filter_by(mac_address=mac_address).first():
        flash('Device already registered', 'danger')
    else:
        new_device = Device(mac_address=mac_address, user_id=current_user.id)
        db.session.add(new_device)
        db.session.commit()
        flash('Device registered successfully', 'success')
    return redirect(url_for('index'))

@app.route('/set_limits', methods=['POST'])
@login_required
def set_limits():
    device = request.form['device']
    temp_min = request.form['tempMin']
    temp_max = request.form['tempMax']
    pressure_min = request.form['pressureMin']
    pressure_max = request.form['pressureMax']
    humidity_min = request.form['humidityMin']
    humidity_max = request.form['humidityMax']

    if temp_min and temp_max:
        temp_message = f"t{temp_min} {temp_max}"
        mqtt_client.publish(f'/esp/{device}/in', temp_message)
    
    if pressure_min and pressure_max:
        pressure_message = f"c{pressure_min} {pressure_max}"
        mqtt_client.publish(f'/esp/{device}/in', pressure_message)
    
    if humidity_min and humidity_max:
        humidity_message = f"w{humidity_min} {humidity_max}"
        mqtt_client.publish(f'/esp/{device}/in', humidity_message)

    return redirect(url_for('index'))

@app.route('/set_temp_limits', methods=['POST'])
@login_required
def set_temp_limits():

    device = request.form['device']
    temp_min = request.form['tempMin']
    temp_max = request.form['tempMax']

    if temp_min and temp_max:
        temp_message = f't{temp_min} {temp_max}'
        mqtt_client.publish(f'/esp/{device}/in', temp_message)

    return redirect(url_for('index'))

@app.route('/set_pressure_limits', methods=['POST'])
@login_required
def set_pressure_limits():
    device = request.form['device']
    pressure_min = request.form['pressureMin']
    pressure_max = request.form['pressureMax']

    if pressure_min and pressure_max:
        pressure_message = f"c{pressure_min} {pressure_max}"
        mqtt_client.publish(f'/esp/{device}/in', pressure_message)

    return redirect(url_for('index'))

@app.route('/set_humidity_limits', methods=['POST'])
@login_required
def set_humidity_limits():
    device = request.form['device']
    humidity_min = request.form['humidityMin']
    humidity_max = request.form['humidityMax']

    if humidity_min and humidity_max:
        humidity_message = f"w{humidity_min} {humidity_max}"
        mqtt_client.publish(f'/esp/{device}/in', humidity_message)

    return redirect(url_for('index'))


@app.route('/set_freq', methods=['POST'])
@login_required
def set_freq():
    device = request.form['device']
    value = request.form['freq']

    if value:
        message = f"{value}"
        mqtt_client.publish(f'/esp/{device}/in', message)
    else:
        flash('Invalid value', 'danger')

    return redirect(url_for('index'))

@app.route('/set_filter', methods=['POST'])
@login_required
def set_filter():
    device = request.form['device']
    value = request.form['filter']

    if value:
        message = f"{value}"
        mqtt_client.publish(f'/esp/{device}/in', message)
    else:
        flash('Invalid value', 'danger')

    return redirect(url_for('index'))


@app.route('/device_data/<mac_address>')
@login_required
def device_data(mac_address):
    device = Device.query.filter_by(mac_address=mac_address, user_id=current_user.id).first_or_404()
    sensor_data = SensorData.query.filter_by(device_id=device.id).order_by(SensorData.time.desc()).limit(1000).all()
    serialized_data = [
        {
            "time": entry.time,
            "temperature": entry.temperature,
            "pressure": entry.pressure,
            "moisture": entry.moisture,
        }
        for entry in sensor_data
    ]
    return render_template('device.html', device=device, sensor_data=serialized_data)

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        user = User.query.filter_by(username=username).first()
        if user and user.password == password:
            login_user(user)
            return redirect(url_for('index'))
        flash('Invalid username or password', 'danger')
    return render_template('login.html')

@app.route('/logout')
@login_required
def logout():
    logout_user()
    flash('You have been logged out.', 'success')
    return redirect(url_for('login'))

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        if User.query.filter_by(username=username).first():
            flash('Username already exists', 'danger')
        else:
            new_user = User(username=username, password=password)
            db.session.add(new_user)
            db.session.commit()
            flash('Registration successful', 'success')
            return redirect(url_for('login'))
    return render_template('register.html')

@app.route('/dashboard', methods=['GET', 'POST'])
@login_required
def dashboard():
    if request.method == 'POST':
        mac_address = request.form['mac_address']
        if Device.query.filter_by(mac_address=mac_address).first():
            flash('Device already registered', 'danger')
        else:
            new_device = Device(mac_address=mac_address, user_id=current_user.id)
            db.session.add(new_device)
            db.session.commit()
            flash('Device registered successfully', 'success')

    devices = Device.query.filter_by(user_id=current_user.id).all()
    return render_template('dashboard.html', devices=devices)

# Removed duplicate route definition
# @app.route('/device/<mac_address>', methods=['GET'])
# @login_required
# def device_data(mac_address):
#     device = Device.query.filter_by(user_id=current_user.id, mac_address=mac_address).first()
#     if not device:
#         flash("Device not found or unauthorized access.", "danger")
#         return redirect(url_for('dashboard'))

#     data = SensorData.query.filter_by(device_id=device.id).order_by(SensorData.time.desc()).limit(100).all()
#     serialized_data = [
#         {
#             "time": entry.time,  # Assuming entry.time is already in milliseconds
#             "temperature": entry.temperature,
#             "pressure": entry.pressure,
#             "moisture": entry.moisture,
#         }
#         for entry in data
#     ]
#     return render_template('device.html', device=device, data=serialized_data)

@app.route('/delete_device', methods=['POST'])
@login_required
def delete_device():
    mac_address = request.form['mac_address']
    device = Device.query.filter_by(user_id=current_user.id, mac_address=mac_address).first()
    if device:
        SensorData.query.filter_by(device_id=device.id).delete()
        db.session.delete(device)
        db.session.commit()
        flash("Device deleted successfully", "success")
    else:
        flash("Device not found or unauthorized access", "danger")
    return redirect(url_for('dashboard'))

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT Broker")
    mqtt_client.subscribe('/esp/+/out')

def on_message(client, userdata, msg):
    print(f"Received message on topic: {msg.topic}: {msg.payload}")
    topic = msg.topic
    payload = json.loads(msg.payload.decode('utf-8'))
    mac_address = topic.split('/')[2]

    with app.app_context():
        device = Device.query.filter_by(mac_address=mac_address).first()

        if device:
            new_data = SensorData(
                device_id=device.id,
                time=payload.get('time'),
                temperature=payload.get('temperature'),
                pressure=payload.get('pressure'),
                moisture=payload.get('moisture'),
            )
            db.session.add(new_data)
            db.session.commit()
        else:
            print(f"No registered device for MAC: {mac_address}")

def clear_database():
    db_path = 'app.db'
    if os.path.exists(db_path):
        os.remove(db_path)
        print(f"Database {db_path} has been deleted.")
    else:
        print(f"Database {db_path} does not exist.")
    
    with app.app_context():
        db.create_all()
        print("Database has been recreated.")

if __name__ == '__main__':
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect('127.0.0.1', 1883, 60)
    mqtt_client.loop_start()
    with app.app_context():
        # db.drop_all()
        db.create_all()
    app.run(debug=True)
