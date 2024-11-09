import socket
import json

def get_monte_carlo_price(host, port):
    # Create a socket connection to the server
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            # Connect to the server
            sock.connect((host, port))

            # Send a basic GET request
            request = "GET / HTTP/1.1\r\nHost: {}\r\n\r\n".format(host)
            sock.sendall(request.encode('utf-8'))

            # Receive data from the server
            response = b""
            while True:
                data = sock.recv(4096)
                if not data:
                    break
                response += data

            # Decode response to UTF-8
            response_text = response.decode('utf-8')

            # Print the entire response to inspect it
            print("Full response:")
            print(response_text)

            # Directly parse the response as JSON
            try:
                json_data = json.loads(response_text)
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON: {e}")
                return

            # Print the parsed JSON response
            print("Monte Carlo Pricing Result:")
            print(f"Option Type: {json_data['option_type']}")
            print(f"Model Type: {json_data['model_type']}")
            print(f"Price: {json_data['price']}")
            print(f"Error (Std Dev): {json_data['error']}")
            print(f"Simulations: {json_data['simulations']}")

        except (socket.error, json.JSONDecodeError) as e:
            print(f"Error: {e}")

# Connect to the server running on localhost at port 8080
if __name__ == "__main__":
    get_monte_carlo_price("localhost", 8080)
