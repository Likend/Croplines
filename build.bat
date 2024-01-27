.\.venv\Scripts\activate.bat
python -m nuitka --standalone --windows-icon-from-ico=asserts\mainicon.ico --mingw64 --output-dir=build --windows-disable-console --include-data-dir=asserts=asserts --include-module=wx._xml app.py

@REM python -m nuitka --standalone --windows-icon-from-ico=asserts\mainicon.ico --mingw64 --output-dir=build  --include-data-dir=asserts=asserts --include-module=wx._xml app.py
