#!/usr/bin/env python3
import asyncio
from bleak import BleakClient, BleakScanner
import struct

import typer

import datetime
import pytz

app = typer.Typer()

CHAR_UUID = "0000ff01-0000-1000-8000-00805f9b34fb"

class scale_mutator:
    def __init__(self,scale):
        self.scale = scale
    def mutate(self,val):
        return val/self.scale

class nop_mutator:
    def mutate(self,val):
        return val

class date_mutator:
    def mutate(self,val):
        return datetime.datetime.fromtimestamp(val,pytz.UTC)

def notification_handler(sender, data):
    keys=("sample_in_past","date","i_slr","i_bat","v_slr","v_bat","valid")
    vals=struct.unpack("<IIHhHHB",data)
    mutators = (
        nop_mutator(),
        date_mutator(),
        scale_mutator(100),
        scale_mutator(100),
        scale_mutator(100),
        scale_mutator(100),
        nop_mutator(),
        )
    vals = [r.mutate(val) for val,r in zip(vals,mutators)]
    print(dict(zip(keys,vals)))


import pprint

async def amain(address,raw_cmd):
    client=BleakClient(address.lower())
    await client.connect()
    await client.start_notify(CHAR_UUID, notification_handler)
    if raw_cmd is not None:
        await client.write_gatt_char(CHAR_UUID, raw_cmd)
    while True:
        await asyncio.sleep(1)



class aread():
    def __init__(self):
        self.read_log = []
        self.read_all = False
        self.read_next = 0
        self.read_evt = asyncio.Event()

    def notification_read_handler(self,sender, data):
        keys=("sample_in_past","date","i_slr","i_bat","v_slr","v_bat","valid")
        vals=struct.unpack("<IIHhHHB",data)
        mutators = (
            nop_mutator(),
            date_mutator(),
            scale_mutator(100),
            scale_mutator(100),
            scale_mutator(100),
            scale_mutator(100),
            nop_mutator(),
            )
        vals = [r.mutate(val) for val,r in zip(vals,mutators)]
        entry = dict(zip(keys,vals))
        if entry["sample_in_past"] == 0:
            return
        if entry['valid'] != 1:
            self.read_all = True
            self.read_evt.set()
            return
        self.read_log.append(entry)
        self.read_next = entry["sample_in_past"]+1
        self.read_evt.set()

    async def amain_read(self,address):
        client=BleakClient(address.lower())
        await client.connect()
        await client.start_notify(CHAR_UUID,self.notification_read_handler)
        await client.write_gatt_char(CHAR_UUID, b"\x01")
        await client.write_gatt_char(CHAR_UUID, struct.pack("<BI",2,1))
        while True:
            await self.read_evt.wait()
            self.read_evt.clear()
            if self.read_all == True:
                break
            await client.write_gatt_char(CHAR_UUID, struct.pack("<BI",2,self.read_next))
        pp = pprint.PrettyPrinter(indent=4)
        pp.pprint(self.read_log)


@app.command()
def read_all(address:str):
    a=aread()
    asyncio.run(a.amain_read(address))

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
def sync_time(address:str,):
    raw_cmd = struct.pack("<BI",3,int(datetime.datetime.utcnow().timestamp()))
    asyncio.run(amain(address,raw_cmd))


@app.command()
def aaa():
    pass

if __name__ == "__main__":
    app()
