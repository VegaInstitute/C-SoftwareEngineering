import numpy as np
import matplotlib.pyplot as plt
from fastapi import FastAPI, Response
import io


app = FastAPI()


def plot_sq_form(a: float, b: float, c: float):
    x = np.linspace(-3, 3, 100)
    y = a * x**2 + b * x + c
    fig, ax = plt.subplots()
    ax.plot(x, y)
    ax.set_xlim((-3, 3))
    ax.set_ylim((-10, 10))
    return fig


@app.get("/plot")
def plot(a: float = 0, b: float = 0, c: float = 0):
    fig = plot_sq_form(a, b, c)
    buf = io.BytesIO()
    fig.savefig(buf, format="png")
    plt.close(fig)
    headers = {'Content-Disposition': 'inline; filename="out.png"'}
    resp = Response(buf.getvalue(), headers=headers, media_type='image/png')
    buf.close()
    return resp
