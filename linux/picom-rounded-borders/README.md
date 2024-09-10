# picom-rounded-borders

This is a simple GLSL shader for enablind rounded window borders in Picom.
It works by sampling color of the original border and adding fillets in the corners.
This method relies on the borders provided by your WM and therefore doesn't require patching anything or installing extra software.

To use the shader run:
```
picom --corner-radius=50 --window-shader-fg=rounded-borders.glsl --backend=glx
```

_Note: border thickness has to be configured in the shader. In my case it's 2px._

### Screenshots

![Screenshot_20240910_174920](https://github.com/user-attachments/assets/ad4c4bf1-bdec-40ed-b716-0960f982c84e)

![Screenshot_20240910_175221](https://github.com/user-attachments/assets/9d57a4d6-a5e7-434b-b82e-c400b0b206a6)
