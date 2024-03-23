# Documentation Instructions

To prepare the local docs for preview:

```bash
$ python -m venv env
$ source env/bin/activate
$ pip install -r requirements.txt
$ `cd $(find env/lib -type d -name sphinxcontrib) && wget https://raw.githubusercontent.com/wpilibsuite/sphinxext-remoteliteralinclude/main/sphinxext/remoteliteralinclude.py`
$ for file in $(find . -name "*.dot"); do dot -Tpng "$file" -o "${file%.*}.png"; done
```

Then build the thing you want:

```bash
make html
```

To see complete list of supported output formats, just run `make`.

When building as HTML, the output can be viewed using

```bash
firefox _build/html/index.html
```

