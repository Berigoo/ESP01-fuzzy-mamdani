import numpy as np
import skfuzzy as fuzz
from skfuzzy import control as ctrl
import matplotlib.pyplot as plt

from websockets.server import serve
from websockets.exceptions import ConnectionClosedOK
import asyncio
import json

# Sebab dan Akibat
suhu = ctrl.Antecedent(np.arange(0, 101, 1), 'suhu')
hum = ctrl.Antecedent(np.arange(0, 101, 1), 'kelembapan')
curah_hujan = ctrl.Consequent(np.arange(0, 265, 1), 'curah hujan')

# Membership function
suhu['dingin'] = fuzz.membership.trapmf(suhu.universe, [0, 0, 18, 27])
suhu['hangat'] = fuzz.membership.trimf(suhu.universe, [23, 29, 32])
suhu['panas'] = fuzz.membership.trapmf(suhu.universe, [30, 35, 100, 100])

hum['kering'] = fuzz.membership.trapmf(hum.universe, [0, 0, 47, 68])
hum['lembab'] = fuzz.membership.trimf(hum.universe, [55, 73, 86])
hum['basah'] = fuzz.membership.trapmf(hum.universe, [78, 98, 100, 100])

curah_hujan['rendah'] = fuzz.membership.trapmf(curah_hujan.universe, [0, 0, 90, 160])
curah_hujan['sedang'] = fuzz.membership.trimf(curah_hujan.universe, [140, 177, 210])
curah_hujan['tinggi'] = fuzz.membership.trapmf(curah_hujan.universe, [190, 265, 265, 265])


# Menghitung aktivasi membership function di nilai tertentu
def Suhu(_nilai):
    dingin = fuzz.interp_membership(suhu.universe, suhu['dingin'].mf, _nilai)
    hangat = fuzz.interp_membership(suhu.universe, suhu['hangat'].mf, _nilai)
    panas = fuzz.interp_membership(suhu.universe, suhu['panas'].mf, _nilai)

    mfPlot2(suhu, ['dingin', 'hangat', 'panas'], 'C', 'membership', 'Suhu', _nilai, 'current')

    return dingin, hangat, panas


def Hum(_nilai):
    kering = fuzz.interp_membership(hum.universe, hum['kering'].mf, _nilai)
    lembab = fuzz.interp_membership(hum.universe, hum['lembab'].mf, _nilai)
    basah = fuzz.interp_membership(hum.universe, hum['basah'].mf, _nilai)

    mfPlot2(hum, ['kering', 'lembab', 'basah'], '%', 'membership', 'Kelembaban', _nilai, 'current')

    return kering, lembab, basah


# rules (Interference)
def Branching(_suhu, _hum):
    rendah = [curah_hujan['rendah'].mf] * 3
    sedang = [curah_hujan['sedang'].mf] * 3
    tinggi = [curah_hujan['tinggi'].mf] * 3

    # IF dingin AND kering THEN rendah
    activate = np.fmin(_suhu[0], _hum[0])
    rendah[0] = np.fmin(activate, curah_hujan['rendah'].mf)
    # IF dingin AND lembab THEN sedang
    activate = np.fmin(_suhu[0], _hum[1])
    sedang[0] = np.fmin(activate, curah_hujan['sedang'].mf)
    # IF dingin AND basah THEN tinggi
    activate = np.fmin(_suhu[0], _hum[2])
    tinggi[0] = np.fmin(activate, curah_hujan['tinggi'].mf)

    # Jika hangat AND kering THEN rendah
    activate = np.fmin(_suhu[1], _hum[0])
    rendah[1] = np.fmin(activate, curah_hujan['rendah'].mf)
    # IF hangat AND lembab THEN sedang
    activate = np.fmin(_suhu[1], _hum[1])
    sedang[1] = np.fmin(activate, curah_hujan['sedang'].mf)
    # IF hangat AND basah THEN tinggi
    activate = np.fmin(_suhu[1], _hum[2])
    tinggi[1] = np.fmin(activate, curah_hujan['tinggi'].mf)

    # Jika panas AND kering THEN rendah
    activate = np.fmin(_suhu[2], _hum[0])
    rendah[2] = np.fmin(activate, curah_hujan['rendah'].mf)
    # IF panas AND lembab THEN sedang
    activate = np.fmin(_suhu[2], _hum[1])
    sedang[2] = np.fmin(activate, curah_hujan['sedang'].mf)
    # IF panas AND basah THEN tinggi
    activate = np.fmin(_suhu[2], _hum[2])
    tinggi[2] = np.fmin(activate, curah_hujan['tinggi'].mf)

    rendah_final = np.fmax(rendah[0], np.fmax(rendah[1], rendah[2]))
    sedang_final = np.fmax(sedang[0], np.fmax(sedang[1], sedang[2]))
    tinggi_final = np.fmax(tinggi[0], np.fmax(tinggi[1], tinggi[2]))

    b0 = [np.zeros_like(rendah_final), np.zeros_like(sedang_final), np.zeros_like(tinggi_final)]
    b1 = [rendah_final, sedang_final, tinggi_final]
    interferencePlot(curah_hujan, ['rendah', 'sedang', 'tinggi'], 'curah hujan', 'membership', 'interference', b0, b1)

    return rendah_final, sedang_final, tinggi_final


# deffuzification untuk mencari nilai curah hujan
def defuzz(_curah_hujan):
    aggregated = np.fmax(_curah_hujan[0], np.fmax(_curah_hujan[1], _curah_hujan[2]))

    Curah_Hujan = fuzz.defuzzify.centroid(curah_hujan.universe, aggregated)
    Curah_Hujan_activation = fuzz.interp_membership(curah_hujan.universe, aggregated, Curah_Hujan)

    deffuzificationPlot(aggregated, ['rendah', 'sedang', 'tinggi'], Curah_Hujan, Curah_Hujan_activation)

    return Curah_Hujan, Curah_Hujan_activation


def calculate(_suhu, _hum):
    SUHU = Suhu(_suhu)
    HUM = Hum(_hum)
    CURAH_HUJAN = Branching(SUHU, HUM)

    hasil = defuzz(CURAH_HUJAN)

    return hasil

def mfPlot(mf, category, xlabel, ylabel, title):
    fig, ax = plt.subplots()
    for key in category:
        ax.plot(mf.universe, mf[key].mf, label=key)
    ax.set(xlabel=xlabel, ylabel=ylabel, title=title)
    ax.legend(loc="upper right")
    fig.savefig(title)

def mfPlot2(mf, category, xlabel, ylabel, title, val, vallabel):
    fig, ax = plt.subplots()
    for key in category:
        ax.plot(mf.universe, mf[key].mf, label=key)
    ax.vlines(val, 0, 1, colors='r', linestyles='dashed', label=vallabel)
    ax.set(xlabel=xlabel, ylabel=ylabel, title=title)
    ax.legend(loc="upper right")
    fig.savefig(title)
    clear()
    plt.close(fig)


def interferencePlot(mf, category, xlabel, ylabel, title, between0, between1):
    fig, ax = plt.subplots()
    i = 0
    for key in category:
        ax.plot(mf.universe, mf[key].mf, label=key, alpha=0.8, linestyle='dashed')
        ax.fill_between(mf.universe, between0[i], between1[i], alpha=0.7)
        i += 1
    ax.set(xlabel=xlabel, ylabel=ylabel, title=title)
    ax.legend(loc="upper right")
    fig.savefig(title)
    clear()
    plt.close(fig)

def deffuzificationPlot(agregated, category, crisp, membership_crisp):
    fig, ax = plt.subplots()
    for key in category:
        ax.plot(curah_hujan.universe, curah_hujan[key].mf, label=key, alpha=0.8, linestyle='dashed')
    ax.fill_between(curah_hujan.universe, np.zeros_like(agregated), agregated, facecolor="Orange", alpha=0.7)
    ax.vlines(crisp, 0, membership_crisp, colors='r', linestyles='dashed', label='center')
    ax.set(xlabel='Curah Hujan', ylabel='membership', title='defuzzification (centroid)')
    ax.legend(loc="upper right")
    fig.savefig('defuzzification')
    clear()
    plt.close(fig)


def clear():
    plt.cla()
    plt.clf()




# mfPlot(suhu, ['dingin', 'hangat', 'panas'], 'suhu', 'membership', 'Suhu')
# mfPlot(hum, ['kering', 'lembab', 'basah'], 'kelembaban', 'membership', 'Kelembaban')
# mfPlot(curah_hujan, ['rendah', 'sedang', 'tinggi'], 'curah hujan', 'membership', 'Curah Hujan')
# plt.show()

# suhu.view()
# hum.view()
# curah_hujan.view()
#
# plt.show()
#
# # testing
# import pandas as pd
# temp = [24, 23.3, 23.7, 24.1, 24.6, 24.1, 23.7, 23.9, 24.7, 25.5, 24.8, 24.6]
# humidity = [76, 80, 80, 79, 77, 77, 72, 70, 64, 65, 79, 78]
# curah_hujan_riil = [67, 111, 200, 276, 269, 90, 24, 30, 18, 62, 239, 365]
# result = []
# for i in range(len(temp)):
#     result.append(calculate(temp[i], humidity[i])[0])
# error_percentage = np.multiply(np.divide(abs(np.subtract(curah_hujan_riil, result)), result), 100)
# MAE = 1/len(error_percentage) * np.sum(error_percentage)
# frame = pd.DataFrame(np.column_stack((temp, humidity, result, curah_hujan_riil, error_percentage)),
#                      columns=['Suhu (C)', 'Humidity (%)', 'Curah Hujan (mm)', 'Curah Hujan Asli (mm)', 'error (%)'])
# print(frame)
# print(f"Rata-rata persentase error: {round(MAE, 2)}%")
#
connected = set()
monitor = set()

async def handle_websocket(websocket):
    msg = await  websocket.recv()
    if msg == 'monitor':
        monitor.add(websocket)
        print(f"monitor registered")
    else:
        connected.add(websocket)
        print(f"non monitor registered")
    while True:
        try:
            message = await websocket.recv()
        except ConnectionClosedOK:
            if websocket in connected:
                connected.remove(websocket)
                print(f"client disconnected {websocket}")
                break
            elif websocket in monitor:
                monitor.remove(websocket)
                print(f"monitor disconnected {websocket}")
                break
        data = json.loads(message)
        res = calculate(data['suhu'], data['hum'])

        sendData = {"curah_hujan": res[0], "membership": res[1], "suhu": data['suhu'], "hum": data['hum']}
        filename = ['Suhu.png', 'Kelembaban.png', 'interference.png', 'defuzzification.png']
        for key in filename:
            with open(key, 'rb') as img:
                encoded = base64.b64encode(img.read()).decode('utf-8')
                sendData[key] = encoded
        for mon in monitor:
            # print('send')
            await mon.send(json.dumps(sendData))


async def main():
    async with serve(handle_websocket, "192.168.1.5", 8765):        
        await asyncio.Future()


asyncio.run(main())
