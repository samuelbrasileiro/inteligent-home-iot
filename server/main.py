from typing import Union
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.dates

from fastapi import FastAPI
from pydantic import BaseModel
import firebase_admin
from firebase_admin import db

class Log(BaseModel):
    redKW: Union[int, None] = None
    yellowKW: Union[int, None] = None
    greenKW: Union[int, None] = None
    resistorKW: Union[int, None] = None

cred_obj = firebase_admin.credentials.Certificate('credentials/serviceAccountKey.json')
default_app = firebase_admin.initialize_app(cred_obj, {
	'databaseURL': 'https://smart-home-e8e9e-default-rtdb.firebaseio.com/'
	})

app = FastAPI()

ref = db.reference("/logs")

@app.get("/analyze")
async def root():
    logs = ref.order_by_child('timestamp').limit_to_last(20).get()
    array = [item[1] for item in list(logs.items())]

    redKWs = ('redKW', [log['redKW'] for log in array])
    yellowKWs = ('yellowKW', [log['yellowKW'] for log in array])
    greenKWs = ('greenKW', [log['greenKW'] for log in array])
    resistorKWs = ('resistorKW', [log['resistorKW'] for log in array])
    
    keyed = [redKWs, yellowKWs, greenKWs, resistorKWs]

    dates = matplotlib.dates.date2num([datetime.fromtimestamp(log['timestamp']) for log in array])
    
    plt.xlabel('Tempo ->')
    plt.ylabel('KW gasto ->')
    for k in keyed:
       plt.plot_date(dates, k[1], '-', label=k[0])
    plt.legend()
    print("dd")
    plt.savefig('relatories/relatory'+ str(datetime.now()) +'.pdf')
    print("ee")
    return {"logs": array}

@app.post("/logs/")
async def create_item(log: Log):
    dict = log.dict()
    dict["timestamp"] = datetime.now().timestamp()
    ref.push().set(dict)
    return {}