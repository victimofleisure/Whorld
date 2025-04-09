## Version 2 of [Whorld](https://victimofleisure.github.io/Whorld/) is being alpha tested! See below.

Whorld is a free, open-source visualizer for sacred geometry. It uses math to create a seamless animation of mesmerizing psychedelic images. You can VJ with it, make unique digital artwork with it, or sit back and watch it like a screensaver.

Whorld's visualization is controlled via parameters which can be adjusted manually, or modulated by oscillators. A given setting of the parameters and their oscillators form a patch. You can use the demo patches, or create your own.

Patches can be grouped together in a playlist. You can play patches manually, or via MIDI, ~~or Whorld can auto-play them for you. Whorld also includes a crossfader, which allows you to interpolate between two patches (AKA "tweening").~~ [not yet in V2]

Whorld is optimized for VJing. All of its functions are available in full-screen mode, via shortcut keys, mouse or trackball, and MIDI controllers. ~~Whorld can be synchronized to music, using tap tempo, or MIDI clocks.~~ [not yet in V2]

**Whorld V2 is in alpha test**

It has been nearly **twenty years** since the last release. The main design goals of Whorld V2 are listed below.

1. Improve *rendering throughput* by leveraging Direct2D’s hardware acceleration. [done]
2. Implement *anti-aliasing* (line smoothing), which is a core feature of Direct2D.  [done]
3. Improve *accuracy* by drawing in floating-point coordinates, another Direct2D feature.  [done]
4. Modernize the user interface via *dockable panes* and similar enhancements. [done]
5. Move rendering to a worker thread, to avoid glitches and tearing. [done]
6. Redo the MIDI mapping, to improve usability and flexibility. [done] 
7. Streamline the application, preserving what’s valuable and discarding what isn’t. [in progress]
8. Eventually, implement quad mirroring, transparency (alpha) and other visual effects.

If you wish to be an official _alpha tester_, please contact the author via their website, social media, or GitHub. Note the system requirements below. 

**Requirements:**
* PC running Windows 10 or 11 with DirectX 12 (or higher)
* Whorld could conceivably run on Windows 8 or even Windows 7 if the necessary service packs are applied, but performance will likely suffer. Direct2D 1.1 is **required**.
* Whorld may work satisfactorily on a Mac via virtualization software such VMWare or Parallels, but this is unproven. Note that virtualizer-induced misbehaviors will be considered _features_, not bugs.

V2 has its own issues list, [HERE](https://victimofleisure.github.io/Whorld/Whorld2-issues.html). V2 does not necessarily agree with the V1 documentation, and this will be addressed though not immediately.

“There’s the way it ought to be. And there’s the way it is.” —Sgt. Barnes, _Platoon_

“I mean it like it is, like it sounds.” —Mike, _Twin Peaks_, “Cooper’s Dreams”

