import requests
from argparse import ArgumentParser

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("hostname")
    parser.add_argument("method", nargs="?", default="HEAD")
    args = parser.parse_args()

    response = requests.request(args.method, "http://" + args.hostname)
    print(response.status_code)
    print(response.headers)
    print("----------------")
    print(response.text)
    print("----------------")
    print(response.content)
    print("----------------")
    print(response.raw)
