import http.server
import socketserver
import mimetypes

# Add wasm MIME type
mimetypes.add_type('application/wasm', '.wasm')

PORT = 8000
Handler = http.server.SimpleHTTPRequestHandler

try:
    with socketserver.TCPServer(("", PORT), Handler) as httpd:
        print(f"Serving at port {PORT}")
        httpd.serve_forever()
except KeyboardInterrupt:
    print("\nShutting down server.")
