#!/usr/bin/env python
# coding: utf-8

import socket

<<<<<<< HEAD
hote = "192.168.1.13"
=======
hote = "192.168.1.16"
>>>>>>> eb699f1be2664e0a953dc6960d3dec5dda5085b4
port = 15555

socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socket.connect((hote, port))
print("Connection on {}".format(port))

socket.send(b"Hey my name is Olivier!")

print("Close")
socket.close()
