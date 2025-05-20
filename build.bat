.\.venv\Scripts\activate.bat
python -m nuitka --standalone --windows-icon-from-ico=assets\mainicon.ico --mingw64 --output-dir=build --windows-disable-console --include-data-dir=assets=assets --include-module=wx._xml --include-module=cv2.typing app.py

@REM python -m nuitka --standalone --windows-icon-from-ico=assets\mainicon.ico --mingw64 --output-dir=build  --include-data-dir=assets=assets --include-module=wx._xml --include-module=cv2.typing app.py
