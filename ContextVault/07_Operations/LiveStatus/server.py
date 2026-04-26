from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HOST = "127.0.0.1"
PORT = 8765
ROOT = Path(__file__).resolve().parent


def main() -> None:
    handler = partial(SimpleHTTPRequestHandler, directory=str(ROOT))
    server = ThreadingHTTPServer((HOST, PORT), handler)
    print(f"SM-13 Live Status dashboard: http://{HOST}:{PORT}/index.html")
    print(f"Serving read-only files from: {ROOT}")
    server.serve_forever()


if __name__ == "__main__":
    main()
