## How to contribute to the project
I am more or less at the point in this project where new features are unlikely to be added by myself.

Please if you would like to contribute right now I am focused on improving the following:
* **Security** - I am no security expert however I can see there is room for improvement in this sector, this is something I would like to focus on first however unlikely it is somebody is to break this security through obscurity is no way to go.
Instead of transmitting the password with every request perhaps at some mount design a proprietary application for IOS/Android similar to my Tizen app that includes something such as rolling codes, ticket-based authentication... if these are even worthwhile. :shrug:
* Modularity - I would like to look into making the current modules independent of each other, so if you only wanted the alarm module you would not require the engine module.
* Code Cleanliness - While my current version is significantly better than the one liner that I originally drafted this from I feel that there are some areas where class and function names do not properly do what they are named, just some general laziness on my part.
Some functions say that they do something but really they are meant to work in conjunction with other functions and are being improperly exposed, causing issues such as variables being set improperly making future revision more challenging.

**If you could contribute in these areas please feel free to open a pull request a with details about what your change entails and if you see any unforseen compatibility issues etc.**
