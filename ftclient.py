#!/usr/bin/python

# Mark Rushmere
# CS 372
# Project 2
# File Transfer client sends commands to ftserver (ftserver.c)




import sys
import socket
import time
import getopt

BUFFER_SIZE = 1024

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
			print("Invalid option")

	# Set up server socket
	serverSock = setSocket(serverName, serverPort)
		
	# Send the command
	serverSock.send(com)

	serverDataSock = setSocket(serverName, serverDataPort)

	# get response from server 
	res = serverSock.recv(512)

	if res != 'OK':
		print "invalid command sent"
	else:
		print "response OK received"
		# Set up data socket
		resHandler(serverSock, serverDataSock, com, fileName)

	serverSock.close()



def resHandler(serverSock, serverDataSock, com, fileName):
	print "in resHandler"
	if(com[0] == "l"):
		print "list command\n"
		directory = ''
		buf = '\n'
		while buf != '':
			buf = serverDataSock.recv(BUFFER_SIZE)
			directory += buf
		print(directory)


	elif(com[0] == "g"):
		print "get command\n"
		print "receiving" + '"' + fileName + '"'
		contents = ''
		buf = '\n'
		while buf != '':
			buf = serverDataSock.recv(BUFFER_SIZE)
			contents += buf
		with open(fileName, "w") as f:
			f.write(contents)


	else:
		print("error")
		fContents = ""
		buf = "\n"
		while buf != "":
			buf = serverDataSock.recv(BUFFER_SIZE)
			fContents += buf
		with open(fileName, "w") as f:
			f.write(fContents)
		f.close()
		print "transfer complete"
	serverDataSock.close()

def setSocket(host, port):
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	time.sleep(0.1)
	try:
		serverSocket.connect((host, int(port)))
		print("connected successfully")
	except:
		print "error connecting"
		sys.exit()	

	print("socket set successfully")
	return serverSocket



if __name__ == "__main__":
	main()