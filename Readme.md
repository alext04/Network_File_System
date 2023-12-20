
# Network-Based File Management System 

The network-based file management system comprises three key components: the **Client Server**, the **Naming Server**, and the **Storage Server**. 

## Client Server

The **Client Server** allows users to interact with the file management system. It handles user requests, by taking input for Writing a File/Folder, Reading a File, Deleting a File/Folder, Creating a File/Folder, Getting Additional Information. 

<br>make br U_Name=
<br>
Where U_NAME = Client Username
## Naming Server

The **Naming Server** serves as the central coordination point of the file system. It is responsible for managing the directory structure and maintaining essential information about file locations. The Naming Server begins accepting client requests. It becomes the central point through which clients access and manage files and directories within the network file system.

- **Initializing the Naming Server**
 <br> make br
- **Redudancy**

- **Efficient Search**

## Storage Server

The **Storage Server** is responsible for managing the actual storage of files. It stores and retrieves data based on requests from the Client Server and works in collaboration with the Naming Server to locate files efficiently.

- **Initializing the Storage Server(s)**  

<br>make br SS= CLIENT= NAMING= SERVER=

<br>SS=Storage Server Id 
<br>CLIENT - Port to listen for Client Requests 
<br>NAMING - Port to listen for Naming Server Requets
<br>SERVER - Port to listen for inter Storage Server Requests

## Functions Implemented

- **Reading a File:**  
FORMAT - readfile path  
The client sends a request to the naming server from the client server for the client id. to get the complete path. Using the complete path a command is sent to the storage server for execution. The Storage Server responds to the request by sending the contents of the file as a response to the client and the contents are displayed to the user.

- **Writing a File:**  
FORMAT - writefile path content  
The client sends a request to the naming server from the client server for the client id. to get the complete path. Using the complete path, a command along with the content to write is sent to the storage server for execution. The Storage Server responds to the request when it is successfullly written to the intended location.

- **Getting Additional Information:**  
FORMAT - fileinfo path  
The client sends a request to the naming server from the client server for the client id. to get the complete path. Using the complete path a command is sent to the storage server for execution. The Storage Server responds to the request by sending the information about the file as a response to the client and the information are displayed to the user.

- **Creating a File/Directory:**   
FORMAT - createfile path  
FORMAT - createdir path  
The client sends the naming server the path of the file/folder it wants to create, the naming server adds the clientID to the path and sends it to the Storage Server. The Server sends a reponse on executing the action. 

- **Deleting a File/Directory:**  
FORMAT - deletefile path  
FORMAT - deletedir path  
The client sends the naming server the path of the file/folder it wants to copy, the naming server adds the clientID to the path and sends it to the Storage Server. The Server sends a reponse on executing the action.

- **Copying a File:**  
FORMAT - copyfile source_path dest_path     
The client sends the naming server the path of the file it wants to copy, the naming server adds the clientID to the path and sends it to the Storage Server. The Server sends a reponse on executing the action. To execute the copy the storage server has an inter storage server port to communicate between different storage servers to copy from one server to the other.  When copying a folder, we use **tar** to compress the directory and transfer it over the network and build it again in the new destination.

- **Copying a Directory:**  
FORMAT - copydir source_path  dest_path    
The client sends the naming server the path of the file it wants to copy, the naming server adds the clientID to the path and sends it to the Storage Server. The Server sends a reponse on executing the action. When copying a folder, we use **tar** to compress the directory and transfer it over the network and build it again in the new destination.













