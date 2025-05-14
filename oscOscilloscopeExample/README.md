oscOscilloscopeExample
===============

An example that reads data from an OSC stream and plots it in an oscilloscope window. 

<img src="../assets/oscOscilloscope.png" width="650">

## Basic Usage:

- Edit oscInputSettings.xml
  - Change the port to listen to your OSC stream 
  - Change the OSC addresses to match the incoming OSC addresses
  - Reload oscInputSettings by hitting (capital) 'P'
- Edit ofxOscilloscopeSettings.xml 
  - If desired, change how the oscilloscope is displayed
  - Note that the \<plotId\> in ofxOscilloscopeSettings.xml should match the \<output\> field in oscInputSettings.xml to display a specific OSC stream in a specific oscilloscope plot
  - Reload ofxOscilloscopeSettings with 'L'
- Run
