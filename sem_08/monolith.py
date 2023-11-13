import numpy as np
import matplotlib.pyplot as plt
from fastapi import FastAPI, Response
from fastapi.responses import HTMLResponse
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


@app.get("/", response_class=HTMLResponse)
def read_root():
    html_content = """
    <html>
        <head>
            <title>Hi</title>
        </head>
        <body>
            <h1>I can plot square forms!</h1>
            <br>
            <form action="/plot">
                <label for="a">x^2 coeff:</label><br>
                <input type="text" id="a", name="a", value="1"><br>
                <label for="b">x coeff:</label><br>
                <input type="text" id="b", name="b", value="0"><br>
                <label for="c">free coeff:</label><br>
                <input type="text" id="c", name="c", value="-1"><br>
                <br>
                <input type="submit" value="Plot">
            </form>
        </body>
    </html>
    """
    return HTMLResponse(content=html_content, status_code=200)


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
