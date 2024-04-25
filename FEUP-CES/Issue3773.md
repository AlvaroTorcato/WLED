# Rearranging presets [#3773](https://github.com/Aircoookie/WLED/issues/3773)

## Issue Description and Requirements

From the original issue description we can reed the following:

**Is your feature request related to a problem? Please describe.**

Rearranging presets requires juggling IDs and is very tiresome.

**Describe the solution you'd like**

An option to rearrange presets and have WLED automatically update their IDs to reflect the changes.
Changes could be saved locally until you applied them to avoid issues.

**Additional context**

I use my WLED setup most of the day since it's my preferred light source in my room. I control it with an IR remote most of the time and because of that it's important that my presets are well organized for ease of access when cycling between them.

The current version of WLED allows the user to see the Presets ordered alphabetically or by id. The author of the issue wants to change the ID of a Preset in order to rearrange the order of the presets. Before this fix, if you try to change the ID of a preset, it will just create a copy of the Preset and override the preset that had that ID. 
This allows changing the Preset ID without overriding the old Preset that had that ID, shifting the other Preset IDs in order to allow the new one to fit in between.

## Changes to the source code files

The changes on the source code can be checked on the [Pull Request #3929](https://github.com/Aircoookie/WLED/pull/3929). 

The fix was applied in the ``wled00/presets.cpp``:

![img.png](img.png)

![img_1.png](img_1.png)

![img_2.png](img_2.png)

![img_3.png](img_3.png)


## Desing of the fix