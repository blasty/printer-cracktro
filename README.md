# printer cracktro

My Pwn2Own entries for Pwn2Own Toronto 2022 targeted two different printers.
One is Linux based (Lexmark) and other is DryOS based (CANON).

After pwning the software on the printer the entries would display a little 
oldschool crack-intro animation thing to celebrate/illustrate the victory.

I got quite a bit of inquiries about "how does the animation thing work???".
Obviously, the  answer is pretty underwhelming.. it is just a couple 100 lines 
of poorly written C code that push pixels to the framebuffers/LCD. :-)

There was no time/actual hardware (although the printers do have some kind 
of piezo beepers) to add a sweet chiptune, sorry. ;-)

Anyway, here it is for those who want to study it for whatever reason. The
same codebase can be built for the following targets:
* Canon (firmware 11.04, update specs/canon_gadgets.ld for other versions)
* Lexmark
* SDL2
* [WebAssembly](https://haxx.in/files/canon_wasm.html)! (through emscripten)

Everything is distributed as-is, don't expect support/updates.

Enjoy!

-- blasty <peter@haxx.in>
