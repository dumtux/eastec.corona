#!/usr/bin/env python3
import asyncio
from bleak import BleakClient, BleakScanner
import struct

import typer

app = typer.Typer()

CHAR_UUID = "0000ff01-0000-1000-8000-00805f9b34fb"

def notification_handler(sender, data):
    keys=("sample_in_past","i_slr","i_bat","v_slr","v_bat","valid")
    vals=struct.unpack("<IHhHHB",data)
    ratio = ("I",100,100,100,100,"I")
    vals = [val/r if r != "I" else val for val,r in zip(vals,ratio)]
    print(dict(zip(keys,vals)))


async def amain(address,raw_cmd):
    client=BleakClient(address.lower())
    await client.connect()
    await client.start_notify(CHAR_UUID, notification_handler)
    if raw_cmd is not None:
        await client.write_gatt_char(CHAR_UUID, raw_cmd)
    while True:
        await asyncio.sleep(1)

@app.command()
def set_led(address:str, colour: str):
    raw_cmd = struct.pack("<BI",0,int(colour,16))
    asyncio.run(amain(address,raw_cmd))

@app.command()
def latch_data(address:str):
    raw_cmd = struct.pack("<B",1)
    asyncio.run(amain(address,raw_cmd))

@app.command()
def get_sample(address:str, sample: int):
    raw_cmd = struct.pack("<BI",2,int(sample))
    asyncio.run(amain(address,raw_cmd))

@app.command()
def just_read(address:str,):
    asyncio.run(amain(address,None))



@app.command()
def aaa():
    pass

if __name__ == "__main__":
    app()
