#####################################################################
# Bot template for Autonomous Mortar Senior Design Project
# Needed packages: discord.py, cryptography
# Developed By: Ty Frame and Benjamin Jones
#####################################################################

from datetime import datetime
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.serialization import load_pem_public_key
from cryptography.hazmat.backends import default_backend
from multiprocessing import Process, Manager
from multiprocessing.managers import BaseManager

import sys
import discord
from discord.ext import commands
import time
import decimal
import re
import asyncio
import base64
import os
import socket

key = sys.argv[1]	# Key for discord bot
myRole = "RCC"	# Role for this bot
client = commands.Bot(command_prefix=myRole)

# HELP 
argumentDict = {
    "from":"The identifier of the sender. Not inteded to be used",
    "command":"The command requested to be executed.",
    "sudo":"The Debug key that will allow a user to call a command. NEEDED FOR ANY DEBUG INTERACTION!",
    "response to":"The command that the deivce is responding to. NOT INTENDED FOR DEBUG USE!",
    "answer":"The actual response of the device for the command it is responding to. NOT INTENDED FOR DEBUG USE!",
    "error":"Used to display errors when parsing messages. NOT INTENDED FOR DEBUG USE!"
    }

##########Dictionaries##########
# Keys
keysDict = {
    "debug":"opensaysme", # Decide on key to verify debug commands
    "mypub":None,
    "mypriv":None,
    "marker":None,
    "turret":None,
    "rcc":None
    }

# Generic Commands
commandDict = {
    "ping":"Used to check if the marker has connected to the discord server.",
    "get ip":"Used to get local ip of the device for ssh purposes.",
    "toggle encryption":"Allows users to see device communication for debug purposes.",
    "get public key":"Will return the public key for the device that the message is directed towards.",
    "toggle message deletion":"Will determine whether recieved messages are deleted immediately or not."
    }

# Update command list with role specific commands
commandDict.update({

    })

##########OBJECTS##########   
class messageQueue(object):
	def __init__(self):
		self.queue = []	# Queue of message objects
		self.newMessageFlag = 0 # 0 for no new messages, 1 for new message in queue
		self.encryption = True # Where we store the value for is we are encrypting or not
		self.delRecMess = True # Boolean for saying whether or not we are clearing the discord chat after a recieved message has been handled
	
		
	def setDel(self,d):
		self.delRecMess = d
		
	def getDel(self):
		return self.delRecMess
		
	def addMessage(self,t,inTo):
		mess = message(t,inTo)
		self.queue.append(mess)
		self.newMessageFlag = 1
		
	def getNewMessageFlag(self):
		return self.newMessageFlag
		
	def setNewMessageFlag(self,f):
		self.newMessageFlag = f
		
	def getMessage(self, i):
		return self.queue[i]
		
	def getQSize(self):
		return len(self.queue)
		
	def popIndex(self,i):
		self.queue.pop(i)
		
	def getEncryptionState(self):
		return self.encryption
		
	def setEncryptionState(self,s):
		self.encryption = s
		
	def handled(self,i):
		self.queue[i].setFlag(1)
   
class message(object):
	def __init__(self, t, inTo):
		self.text = t	# Content of the message
		self.to = inTo	# Where the message is going
		self.flag = 0	# A message might be 'handled' without being removed from the queue. This is to held determine if it has been 'handled'
	
	def getTo(self):
		return self.to
		
	def getFlag(self):
		return self.flag
		
	def getText(self):
		return self.text
		
	def setFlag(self,f):
		self.flag = f
	
class parsedMessage(object):
	def __init__(self, message = None, r = None, c = None):
		self.reciever = None
		self.contents = None
		self.sender = None
		self.command = None
		self.responseTo = None
		self.answer = None
		self.error = None
		self.sudo = None
		if message !="":
			# split the recieved message based on "[" to get the recipient
			# assigns self.reciever
			pattern = "(.*?)\s*\["
			self.reciever = re.search(pattern,message)
			if(self.reciever):
				self.reciever = self.reciever.group(1).strip()
			else:
				self.reciever = "no reciever"
			
			# this sets the contents of the message to the "recievedMessage" object
			pattern = "\\[(.*?)\\]"
			self.contents = re.search(pattern,message)
			if(self.contents):
				self.contents = self.contents.group(1).strip()
			else:
				self.contents = "no message"
		elif r.lower() == myRole.lower():
			self.reciever = r.upper()
			self.contents = keysDict["mypriv"].decrypt(c, padding.OAEP(mgf=padding.MGF1(algorithm=hashes.SHA256()),algorithm=hashes.SHA256(),label=None)).decode()
		else:
			self.reciever = "not me"
			return
	
		# search the contents of the message to see what the command is 
		# assigns self.command
		pattern = "Command:(.*?)(,|$)"
		self.command = re.search(pattern, self.contents)
		if self.command:
			self.command = self.command.group(1).strip().lower()
		else:
			self.command = "no command"
	
		# search the contents of the message to see what the response is 
		# assigns self.responseTo
		pattern = "Response To:(.*?)(,|$)"
		self.responseTo = re.search(pattern, self.contents)
		if (self.responseTo):
			self.responseTo = self.responseTo.group(1).strip().lower()
		else:
			self.responseTo = "no response"
		
		# search the contents of the message to see what the answer is 
		# assigns self.answer
		pattern = "Answer:(.*?)(,|$)"
		self.answer = re.search(pattern, self.contents)
		if (self.answer):
			self.answer = self.answer.group(1).strip()
		else:
			self.answer = "no answer"
		
		# search the contents of the message to see what the error is 
		# assigns self.error
		pattern = "Error:(.*?)(,|$)"
		self.error = re.search(pattern, self.contents)
		if (self.error):
			self.error = self.error.group(1).strip()
		else:
			self.error = "no error"	
			
		# search the contents of the message to see what the error is 
		# assigns self.sudo
		pattern = "Sudo:(.*?)(,|$)"
		self.sudo = re.search(pattern, self.contents)
		if (self.sudo):
			self.sudo = self.sudo.group(1).strip()
		else:
			self.sudo = "no sudo"	
	
######End Of objects#######

def bot(inMQ):
	            
	@client.event
	async def on_ready():
		keysDict["mypriv"] = rsa.generate_private_key(public_exponent = 65537, key_size = 2048, backend = default_backend())	# Generate private key on ready
		keysDict["mypub"] = keysDict["mypriv"].public_key()	# Generate public key from the private key
		print('We have logged in as {0.user}'.format(client))
		sys.stdout.flush()
		await (client.get_channel(809569313925103640)).send(f"{myRole} is here!")	# Alert the server that this client is there
		await asyncio.sleep(5)
		inMQ.addMessage("Command: Give Control","Turret")

	@client.event
	async def on_message(message):
		cont = message.content
	
		pattern = "(.*?) is here!"
		person = re.search(pattern, message.content)
		if person:
			person = person.group(1).strip().lower()
			keysDict[person] = None
			return
	
		name = None
		hold = None	
		
		if str(message.attachments) != "[]":	# Check to see if the message has an attatchment because we send .pem files containing public keys
			split_v1 = str(message.attachments).split("filename='")[1]
			filename = str(split_v1).split("' ")[0]
			if filename.endswith(".pem"):	# Confirm that the file has .pem ending
				await message.attachments[0].save(fp="{}".format(filename))	# Save the file
				with open(filename, "rb") as key_file:
					keysDict[(filename[0:len(filename)-4]).lower()] = serialization.load_pem_public_key(data=key_file.read(),backend=default_backend()) # Load this person's key into the key dictionary
					key_file.close()
				if inMQ.getQSize() > 0:
					inMQ.setNewMessageFlag(1)	# Reset the message queue's new message flag in case this client was waiting for that key to send a message
				return
			elif filename.endswith(".txt"):
				await message.attachments[0].save(fp="{}".format(filename))	# Save the file
				with open(filename, "rb") as key_file:
					hold = key_file.read()
					key_file.close()
				name = filename[0:len(filename)-4]
	
		pm = parsedMessage(message = message.content, r = name, c = hold) # Use our parsedMessage object to get all the fields of the message as well as decrypt if needed
		if myRole.lower() not in pm.reciever.lower():
			return
		
		if pm.contents == "help":	# Outlining how this bot is used
			hold = f"------Start of {myRole} Help------\n"
			hold = hold + f"The {myRole.lower()} bot looks for messages that include the identifier '{myRole}' before the given arguments that are surrounded by hard brackets. Ex: {myRole}" + "[{args}]\n"
			hold = hold + "Input arguments are given in the format '{argument}:{argument descriptor}'.\n"
			hold = hold + "\nArguments:\n"
			for arg in argumentDict:
				hold = hold + "\t" + arg.title() + ": " + argumentDict[arg] + "\n"
        
			hold = hold + "\nAvailable Argument Descriptors For The Argument: Command\n"	
			for command in list(commandDict.keys()):
				hold = hold + "\t" + command.title() + ": " + commandDict[command] + "\n"
        
			hold = hold + f"-------End of {myRole} Help-------\n"
			inMQ.addMessage(hold,"help")
			return
		elif pm.contents == "{args}":	# The help has an example that triggers the bot, ignoring it
			return
		
		if pm.sender == "no sender":	# Error detection 
			inMQ.addMessage(f"Error: There was no retrievable sender for the last message to {myRole}.","Debug")
			return
		
		if pm.sender == "debug" and pm.sudo != keysDict["debug"]:	# Error detection
			inMQ.addMessage(f"Error: The 'sudo' argument was either missing or wrong.","Debug")
			return
        
		commandList = list(commandDict.keys())
		if pm.command in commandList:	# Where all the functions are implemented from the command dictionary
			if pm.command == commandList[0]:
				inMQ.addMessage(f"Response To: {pm.command.title()}, Answer: {myRole} is here!",pm.sender.capitalize())
			elif pm.command == commandList[1]:
				gw = os.popen("ip -4 route show default").read().split()
				s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
				s.connect((gw[2], 0))
				ipaddr = s.getsockname()[0]
				inMQ.addMessage(f"Response To: {pm.command.title()}, Answer: {ipaddr}",pm.sender.capitalize())
			elif pm.command == commandList[2]:
				inMQ.setEncryptionState(not inMQ.getEncryptionState())
				inMQ.addMessage(f"Response To: {pm.command.title()}, Answer: Encryption set to {inMQ.getEncryptionState() if 'True' else 'False'}.",pm.sender.capitalize())
			elif pm.command == commandList[3]:
				print(f"Sending {myRole} public key!")
				sys.stdout.flush()
				filename = f"{myRole}.pem"
				with open(filename, 'wb') as key_file:
					key_file.write(keysDict['mypub'].public_bytes(encoding = serialization.Encoding.PEM, format = serialization.PublicFormat.SubjectPublicKeyInfo))
					key_file.close()
				await (client.get_channel(809569313925103640)).send(file=discord.File(os.path.realpath(filename)))
			elif pm.command == commandList[4]:
				inMQ.setDel(not inMQ.getDel())
				inMQ.addMessage(f"Response To: {pm.command.title()}, Answer: Message deletion set to {inMQ.getDel() if 'True' else 'False'}.",pm.sender.capitalize())
		elif pm.responseTo is not "no response":
			if message.content == "":
				print(f"RCC[{pm.contents}]")
				sys.stdout.flush()
			else:
				print(f"{message.content}")
				sys.stdout.flush()
		else:
			inMQ.addMessage(f"Error: Unrecognized Command In -> {pm.contents}","Debug")
			
		if inMQ.getDel():
			await message.delete() # Delete the messsage that triggered this on_message function
                
	async def send_message(MQ):
		await client.wait_until_ready()
		while not client.is_closed():
			while not MQ.getNewMessageFlag():	# Loop until new message recieved
				await asyncio.sleep(1)
				
			if MQ.getQSize() > 0:	# Ensure queue size is more than none
				for x in range(MQ.getQSize()-1,-1,-1):	# Move from the higher indexes to the lower
					currentM = MQ.getMessage(x)
					if "release control" in currentM.getText().lower():
						MQ.setEncryptionState(False);
				
					if currentM.getTo() == "help":	# Handels the case that this message is the help text for the bot
						MQ.popIndex(x)
						sys.stdout.flush()
						print(f"{currentM.getText()}")
						sys.stdout.flush()
						await (client.get_channel(809569313925103640)).send(f"{currentM.getText()}")
					elif not inMQ.getEncryptionState() or currentM.getTo() == "Debug":	# If statement for when ecryption is false or the entity recieving this message is Debug
						MQ.popIndex(x)
						sys.stdout.flush()
						print(f"{currentM.getTo()}[From: {myRole}, {currentM.getText()}]")
						sys.stdout.flush()
						await (client.get_channel(809569313925103640)).send(f"{currentM.getTo()}[From: {myRole}, {currentM.getText()}]")
					else:
						keySel=None
						if currentM.getTo().lower() in list(keysDict.keys()):	# Ensure the entity recieving this message is in our key dictionary
							keySel = currentM.getTo().lower()
						else:
							sys.stdout.flush()
							print(f"Debug[From: {myRole}, Error: Unkown Reciever, {currentM.getTo()}]")
							sys.stdout.flush()
							await (client.get_channel(809569313925103640)).send(f"Debug[From: {myRole}, Error: Unkown Reciever, {currentM.getTo()}]")
							continue
							
						if keysDict[keySel] == None and not currentM.getFlag():	# If the recieving entity's key has not been stored and a request hasn't been sent to get said key already
							sys.stdout.flush()
							print(f"{currentM.getTo()}[From: {myRole}, Command: Get Public Key]")
							sys.stdout.flush()
							await (client.get_channel(809569313925103640)).send(f"{currentM.getTo()}[From: {myRole}, Command: Get Public Key]")
							MQ.handled(x)
							continue
						elif not keysDict[keySel] == None:							
							MQ.popIndex(x)
							await asyncio.sleep(6)
							print(f"{currentM.getTo()}[From: {myRole}, {currentM.getText()}]")
							sys.stdout.flush()
							encMessage =  keysDict[keySel].encrypt(f"From: {myRole}, {currentM.getText()}".encode(),padding.OAEP(mgf=padding.MGF1(algorithm=hashes.SHA256()),algorithm=hashes.SHA256(),label=None))	# Encrypt message
							filename = f"{currentM.getTo()}.txt"
							with open(filename, 'wb') as key_file:
								key_file.write(encMessage)
								key_file.close()
							await (client.get_channel(809569313925103640)).send(file=discord.File(os.path.realpath(filename)))
				
				MQ.setNewMessageFlag(0) # Set new message flag to 0 to show that all messages have been handled
        
	client.loop.create_task(send_message(inMQ))	# Start up the send message task
	try:	# Attempt to run the client
		client.run(key)
	except:
		print("Bot was unable to log in.")

if __name__ == "__main__":
	BaseManager.register("messageQueue",messageQueue)
	manager = BaseManager()
	manager.start()
	instMQ = manager.messageQueue()

	p = Process(target=bot,args=[instMQ])
	p.start()
    
    # Do stuff
	sayMe = ""
	print(f"Start of {myRole} Bot.")
	sys.stdout.flush()
	while sayMe.lower() != "end":
		try:
			sys.stdout.flush()
			sayMe = input()
			sys.stdout.flush()
		except EOFError:
			sayMe = ""
		sys.stdout.flush()
		
		if "Get Turret Location" in sayMe:
			instMQ.addMessage("Command: Get Position","Turret")
		elif "Get Marker Location" in sayMe:
			instMQ.addMessage("Command: Get Position","Marker")
		elif "Fire At" in sayMe:
			instMQ.addMessage(sayMe,"Turret")
		elif "end" in sayMe:
			instMQ.addMessage("Command: Release Control","Turret")
		elif "Satellite Count" in sayMe and "Marker" in sayMe:
			instMQ.addMessage("Command: Get Satellite Count","Marker")
		elif "Satellite Count" in sayMe and "Turret" in sayMe:
			instMQ.addMessage("Command: Get Satellite Count","Turret")
    
	p.terminate()
	p.join()
