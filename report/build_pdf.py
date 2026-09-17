import re, markdown, pathlib
from weasyprint import HTML, CSS

src = pathlib.Path('/home/claude/report.md').read_text()

# Display-math blocks -> Unicode HTML. Keyed by a distinctive fragment.
EQ = [
 (r'f(P) = \lambda_0',
  'f(P) = λ₀·f(V₀) + λ₁·f(V₁) + λ₂·f(V₂),&nbsp;&nbsp;&nbsp; λ₀ + λ₁ + λ₂ = 1'),
 (r'y_f = \frac{h}{2}',
  'y<sub>f</sub> = (h / 2) · y<sub>d</sub> + ( y<sub>offset</sub> + h / 2 )'),
 (r'\frac{t_{\text{S3, vert}}}{t_{\text{S3, frag}}}',
  't<sub>S3,vert</sub> / t<sub>S3,frag</sub> &nbsp;=&nbsp; 46.167 / 10.091 &nbsp;=&nbsp; <b>4.58</b>'
  '&nbsp;&nbsp;≈&nbsp;&nbsp; (300,000 / 1,400) ÷ (2,080,000 / 46,000) &nbsp;=&nbsp; 214 / 45 &nbsp;=&nbsp; 4.7'),
 (r'\text{Effective throughput}',
  'effective throughput &nbsp;=&nbsp; (active lanes / subgroup size) × peak throughput'),
 (r'\frac{16}{32} = 0.5',
  '16 / 32 = 0.5 &nbsp;⟹&nbsp; effective = 0.5 × peak &nbsp;⟹&nbsp; t<sub>1px</sub> = <b>2.0 × t<sub>no branch</sub></b>'),
 (r'\text{saving} = 1 - \frac{Vs + Ib}{Is}',
  'saving &nbsp;=&nbsp; 1 − (V·s + I·b)/(I·s) &nbsp;=&nbsp; 1 − 1/r − b/s, &nbsp;&nbsp;&nbsp; r = I / V'),
 (r'32 \times 48 = 1{,}536',
  '32 × 48 = 1,536 registers per warp'),
 (r'\left\lfloor \frac{65{,}536}{1{,}536}',
  '⌊ 65,536 / 1,536 ⌋ = ⌊ 42.67 ⌋ = 42 resident warps'),
 (r'\text{occupancy} = \frac{42}{64}',
  'occupancy = 42 / 64 = <b>65.6 %</b>'),
]

blocks = re.findall(r'\$\$(.+?)\$\$', src, flags=re.S)
out = src
for b in blocks:
    html = None
    for key, rep in EQ:
        if key in b:
            html = rep
            break
    if html is None:
        raise SystemExit('unmapped equation:\n' + b)
    out = out.replace('$$' + b + '$$', '<div class="eq">' + html + '</div>', 1)

if '$$' in out:
    raise SystemExit('leftover $$')

body = markdown.markdown(out, extensions=['tables', 'fenced_code', 'sane_lists', 'md_in_html', 'attr_list'])

CSSTEXT = """
@page { size: A4; margin: 17mm 16mm 18mm 16mm;
        @bottom-center { content: "Lab 06 - Your First Triangle   |   page " counter(page) " of " counter(pages);
                         font: 8pt "Helvetica Neue", Helvetica, sans-serif; color: #8a8f98; } }
html { font-size: 10pt; }
body { font-family: "Helvetica Neue", Helvetica, Arial, sans-serif; color: #1d2025;
       line-height: 1.5; text-align: left; }
h1 { font-size: 20pt; margin: 0 0 2mm; letter-spacing: -0.3pt; }
h1 + p { color: #555b63; font-size: 9.5pt; margin: 0 0 4mm; line-height: 1.45; }
h2 { font-size: 13pt; margin: 8mm 0 2.5mm; padding-bottom: 1.2mm;
     border-bottom: 1.4pt solid #1d2025; break-after: avoid; }
h3 { font-size: 10.5pt; margin: 5mm 0 1.5mm; color: #0b3d62; break-after: avoid; }
p, li { orphans: 2; widows: 2; }
code, pre { font-family: "SF Mono", Menlo, Consolas, monospace; }
code { font-size: 8.6pt; background: #f2f3f5; padding: 0.3mm 0.9mm; border-radius: 1.2mm; }
pre { background: #f7f8fa; border: 0.4pt solid #dfe2e7; border-left: 2pt solid #9aa2ad;
      border-radius: 1.5mm; padding: 2.4mm 3mm; font-size: 8.2pt; line-height: 1.42;
      white-space: pre-wrap; break-inside: avoid; }
pre code { background: none; padding: 0; font-size: inherit; }
table { border-collapse: collapse; width: 100%; margin: 3mm 0; font-size: 8.6pt;
        break-inside: avoid; }
th { background: #eef0f3; text-align: left; font-weight: 600; }
th, td { border: 0.4pt solid #ccd1d8; padding: 1.3mm 2mm; vertical-align: top; }
blockquote { margin: 3mm 0; padding: 2.5mm 3.5mm; background: #fff8e6;
             border-left: 2.5pt solid #e0a800; border-radius: 0 1.5mm 1.5mm 0;
             font-size: 9pt; }
blockquote table { font-size: 8.2pt; background: #fff; }
.eq { margin: 3mm 0; padding: 2.6mm 3mm; background: #f4f7fb; border: 0.4pt solid #d3dde8;
      border-radius: 1.5mm; text-align: center; font-size: 9.6pt; break-inside: avoid; }
img.shot { display:block; width: 74%; margin: 3mm auto 1.5mm; border: 0.4pt solid #dfe2e7;
            border-radius: 1.5mm; }
img.fig  { display:block; width: 100%; margin: 3mm 0 2mm; }
p em { color: #6b7078; }
hr { border: none; border-top: 0.4pt solid #dfe2e7; margin: 6mm 0; }
strong { color: #10131a; }
ul { padding-left: 5mm; } li { margin: 0.8mm 0; }
"""

HTML(string='<meta charset="utf-8">' + body, base_url='/home/claude/').write_pdf(
    '/mnt/user-data/outputs/Lab06-report.pdf', stylesheets=[CSS(string=CSSTEXT)])
print('written')
