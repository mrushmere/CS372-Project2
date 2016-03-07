#!/usr/bin/python

# Mark Rushmere
# CS 372
# Project 2
# File Transfer client sends commands to ftserver (ftserver.c)

import sys
import socket
import time
import getopt
import os


def main():

	if(len(sys.argv) < 5 or len(sys.argv) > 6):
		print("Incorrect usage")
		sys.exit()

	# Set up varibles
	serverName = sys.argv[1]
	serverPort = int(sys.argv[2])
	fileName = ""
	com = ""
	serverDataPort = -5

	# List request 
	if len(sys.argv) == 5:
		if(sys.argv[3] == "-l"):
			serverDataPort = sys.argv[4]
			com = "l" +  " " + serverDataPort
			print(com)
		else:
			print("invalid option")

	# Get request
	elif len(sys.argv) == 6:
		if(sys.argv[3] == "-g"):
			fileName = sys.argv[4]
			serverDataPort = sys.argv[5]
			com = "g" + " " + serverDataPort + " " + fileName
			print(com)
		else:
			fileName = "nofilename"
			print("Invalid filename")

	# Set up server socket
	serverSock = setSocket(serverName, serverPort)
	# Send the command
	sendCom(serverSock, com, serverName, serverDataPort, fileName)
	# Close first socket
	serverSock.close()


def setSocket(host, port):
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	time.sleep(1)
	try:
		serverSocket.connect((host, int(port)))
		print("connected successfully")
	except:
		print "error connecting"
		sys.exit()	

	print("socket set successfully")
	return serverSocket

def sendCom(socket, command, name, dataPort, fileName):
	socket.send(command)
	serverDataSock = setSocket(name, dataPort)
	res = socket.recv(512)
	if res != 'OK':
		print(res) # Error message from server
	else:
		if command[0] == 'l':
			listCom(serverDataSock)
		elif command[0] == 'g':
			fileCom(serverDataSock, fileName)
	serverDataSock.close()

def listCom(serverDataSock):
	print "list command\n"
	directory = ''
	buf = '\n'
	while buf != '':
		buf = serverDataSock.recv(1024)
		directory += buf
	print(directory)

def fileCom(serverDataSock, fileName):
	for file in os.listdir('.'):
		if file == fileName:
			fileName += "duplicate"

	contents = ''
	buf = '\n'
	while buf != '':
		buf = serverDataSock.recv(1024)
		contents += buf
	with open(fileName, "w") as f:
		f.write(contents)


if __name__ == "__main__":
	main()