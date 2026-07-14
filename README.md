<h1 align="center">mmry</h1>
  
<p align="center">
  <em>CLI markdown reminders.</em>
</p>

<p align="center">
    <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="MIT License" />
  <img src="https://img.shields.io/github/last-commit/simon-danielsson/mmry/main?style=flat-square&color=blue" alt="Last commit" />
      <img src="https://img.shields.io/badge/C_version-23-cyan?style=flat-square" alt="C version" />
</p>
  
<p align="center">
  <a href="#info">Info</a> •
  <a href="#install">Install</a> •
  <a href="#usage">Usage</a>
  <br>
  <a href="#screenshots">Screenshots</a> •
  <a href="#license">License</a>
</p>  
  
---
<div id="info"></div>

## Info
  
'mmry' is a small C program built for reminding you about certain things - no SQL
database or json files, just a simple markdown file that is parsed at every launch. Inspired by the Emacs org-mode agenda system.
  
This program is not meant to be executed manually (though you can of course do
that too!) - instead, it is designed to be added to your `.bashrc` (or similar) so that it runs each time you open the terminal.
  
---
<div id="install"></div>

## Install
  
Clone this repository and build from source using bundled `run.py` script.
  
``` bash
./run.py release
```
  
An executable will have been generated inside `./build/release`.
  
---
<div id="usage"></div>
  
## Usage
   
### Setup
     
You will feed the executable with a markdown file. Here is a bash function I've
written to streamline this process (you will obviously have to change the paths
to match the location of the executable/markdown-file on your machine):
``` bash
# ~/.bashrc (or ~/.bash_aliases etc.)

mmry() { "$HOME/dev/c/mmry/build/release/"$(command ls -t "$HOME/dev/c/mmry/build/release/" | head -n 1) "~/dev/c/mmry/test.md"; }
```
  
Then you could also launch this command at startup to stay up-to-date on all your
reminders:
``` bash
# ~/.bashrc

mmry
```
  
### Syntax
    
**There are three different item types:**  
- event: a singular event
- repeat: a repeating event
- todo: an event with a true/false state
  
**The 'lead' variable**  
Each new item can take a 'lead' variable that dictates the amount of days before its
scheduled time that it should start appearing in the 'mmry' list. The
'lead' variable can be omitted and will in that case default to a lead-time of 7 days.  
  
**The 'date' variable**  
The date variable must follow the "YYYY-MM-DD" format - specific times such as
the hour, minutes, seconds, as well as time-zones, are not supported.  
  
**More about todo items**  
Todo items that are set to 'false' (i.e done) will not be printed to the 'mmry' list.  
  
### Examples
    
``` markdown
# It's someone's birthday!
{ repeat(365) date(1992-08-10) lead(30) }

# Do the laundry
{ repeat(14) date(2025-01-01) lead(4) }

# Buy groceries
{ todo(true) date(2025-07-02) }

# A rock concert
{ event date(2027-02-06) lead(45) }

# Write a new blog post
{ todo(false) date(2025-08-02) }
```
    
---
<div id="screenshots"></div>
  
## Screenshots
   
No screenshots added yet.
    
---
<div id="license"></div>
  
## License
  
This project is licensed under the [MIT License](https://github.com/simon-danielsson/mmry/blob/main/LICENSE).  
