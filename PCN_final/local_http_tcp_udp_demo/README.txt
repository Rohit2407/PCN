LOCAL WEBPAGE DOWNLOAD USING TCP AND UDP (C SOCKETS)
===================================================

IMPORTANT:
- TCP example connects to a normal local HTTP server.
- UDP example uses a custom UDP server/client to send the HTML file.
- Standard HTTP normally uses TCP. The UDP demo is NOT HTTP.

FILES:
  client_tcp.c       TCP client that sends HTTP GET /index.html
  udp_server.c       Custom UDP server that sends index.html
  udp_client.c       Custom UDP client that saves the HTML response
  index.html         Sample webpage
  README.txt         This guide

REQUIREMENTS:
  Linux / Kali Linux with gcc and Python 3.

---------------------------------------
PART A: TCP + STANDARD LOCAL HTTP
---------------------------------------

1. Open Terminal 1 in this folder and start the HTTP server:

   python3 -m http.server 8080

2. Open Terminal 2 in the same folder and compile the C client:

   gcc client_tcp.c -o client_tcp

3. Run the client:

   ./client_tcp

4. The client sends:
   GET /index.html HTTP/1.1

5. The response is printed and saved as downloaded_page.html.
   The saved file includes HTTP headers followed by the HTML body,
   because this simple client saves the complete HTTP response.

6. To view the original webpage in a browser, open:
   http://127.0.0.1:8080/index.html

Stop the Python server with Ctrl+C.

---------------------------------------
PART B: CUSTOM UDP HTML TRANSFER
---------------------------------------

Use two terminals in this folder.

1. Compile the UDP server and client:

   gcc udp_server.c -o udp_server
   gcc udp_client.c -o udp_client

2. Terminal 1: start the UDP server:

   ./udp_server

3. Terminal 2: run the UDP client:

   ./udp_client

4. The client sends the request text "GET_HTML".
   The custom UDP server sends the bytes from index.html.
   The client saves them as udp_downloaded_page.html.

IMPORTANT UDP LIMITATION:
- UDP does not provide reliable delivery, ordering, or retransmission.
- This demo sends the webpage in one datagram and is only suitable for
  this small sample. Large webpages can exceed the datagram size and need
  chunking, sequence numbers, acknowledgements, and retransmission logic.
- UDP here is not a standard HTTP web server.

---------------------------------------
COMMON TROUBLESHOOTING
---------------------------------------

- If connect() fails in the TCP client, check that the Python HTTP server
  is already running on port 8080.
- Run all commands from the folder containing index.html.
- If bind() says "Address already in use", stop the old server process or
  change the port consistently in server and client.
- For this example, use 127.0.0.1 because client and server are on the
  same computer.
