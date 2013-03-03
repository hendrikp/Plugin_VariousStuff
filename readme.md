VariousStuff Plugin for CryEngine SDK
=====================================
Various Flownodes from the C++ tutorial series.

Installation / Integration
==========================
Use the installer or extract the files to your CryEngine SDK Folder so that the Code and BinXX/Plugins directories match up.

The plugin manager will automatically load up the plugin when the game/editor is restarted or if you directly load it.

Tutorial Video
==============
* [Youtube tutorial series on Flownode and Plugin creation](http://www.youtube.com/watch?v=W7wHus-bunk&list=PL1DcRWGqhCQRIBhLfd9pyfeyH-GZZobkR)

Flownodes
=========

ValueChannel / Signaler / Transmitter
-----------------------
* ```VariousStuff:ValueChannel:*``` Transmit/Receives values through a channel between multiple flowgraphs (replacement for FGPS Signaler)
  * In ```Channel``` Name of the channel to use (each datatype has independent channels)
  * In ```Value``` Value that will be transmitted to all ValueChannel nodes of the same channel
  * Out ```Value``` Value coming out of the channel
  * Supported Datatypes: Integer, Float(Number), String, Vec3, EntityId, Void/Any

LookAt
----------
* ```VariousStuff:LookAtEntity``` Orients an entity/the player/camera to look at another"
  * In ```EntityId``` Entity to be rotated
  * In ```Look``` Instruct the entity to look at the target
  * In ```Stop``` Instruct the entity to stop looking at the target
  * In ```Target``` Target entity to look at
  * In ```Speed``` Between 0-1 How fast to rotate (0 for instant)
  * In ```Constant``` Set if entity should constantly keep looking at the target

TCP/IP Kinect Client
-----------
* ```VariousStuff:TCPClientKinect``` TCP/IP Client for binary Kinect Data
  * I would recommend to use instead the OSC_Plugin

ValueTransform
----------
* ```VariousStuff:ValueTransform``` Clamp and scale a value
