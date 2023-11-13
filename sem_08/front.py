from fastapi import FastAPI, Response
from fastapi.responses import HTMLResponse
import requests


app = FastAPI()
BACK_END_ENDPOINT = "http://localhost:8001/plot"


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
    resp = requests.get(BACK_END_ENDPOINT, params={"a": a, "b": b, "c": c})
    return Response(resp.content, headers=resp.headers)
