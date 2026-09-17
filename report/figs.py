import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

BLUE, ORANGE = '#2a78d6', '#eb6834'
INK, INK2, MUTED, SURF = '#0b0b0b', '#52514e', '#9a9893', '#fcfcfb'
plt.rcParams.update({
    'font.family': 'DejaVu Sans', 'font.size': 8.5,
    'axes.edgecolor': '#d6d4cf', 'axes.linewidth': 0.8,
    'text.color': INK, 'axes.labelcolor': INK2,
    'xtick.color': INK2, 'ytick.color': INK2,
    'figure.facecolor': SURF, 'axes.facecolor': SURF,
})

# ---------------- Figure 1: Part III stripe sweep ----------------
w = np.array([1, 2, 4, 8, 16, 32, 64, 128])
t = np.array([4.024, 3.925, 3.935, 2.122, 2.022, 2.010, 2.033, 2.015])
flat = 2.007

fig, ax = plt.subplots(figsize=(6.3, 2.9))
ax.axhline(2 * flat, color=MUTED, lw=1.0, ls=(0, (4, 3)), zorder=1)
ax.axhline(flat, color=MUTED, lw=1.0, ls=(0, (4, 3)), zorder=1)
ax.text(1.55, 2 * flat + 0.06, 'Eq. 2 prediction for 1 px  (2.00 × no branch)',
        va='bottom', ha='left', color=INK2, fontsize=7.6)
ax.text(1.55, flat - 0.06, 'no branch  2.007 ms',
        va='top', ha='left', color=INK2, fontsize=7.6)

ax.plot(w, t, color=BLUE, lw=2, marker='o', ms=6, mec=SURF, mew=1.6, zorder=3)
for x, y, lab, dy in [(1, 4.024, '4.024 ms', 10), (8, 2.122, '2.122', 11)]:
    ax.annotate(lab, (x, y), textcoords='offset points', xytext=(0, dy),
                ha='center', fontsize=8, color=INK, fontweight='bold')

ax.axvspan(6.5, 9.8, color=BLUE, alpha=0.07, zorder=0)
ax.annotate('penalty gone at 8 px\n= the subgroup is 8 px wide', (8, 2.122),
            textcoords='offset points', xytext=(16, 34), ha='left',
            fontsize=7.6, color=INK2,
            arrowprops=dict(arrowstyle='-', color=MUTED, lw=0.8,
                            connectionstyle='arc3,rad=-0.2'))

ax.set_xscale('log', base=2)
ax.set_xticks(w)
ax.set_xticklabels([str(v) for v in w])
ax.set_xlim(0.85, 165)
ax.set_ylim(1.72, 4.55)
ax.set_xlabel('stripe width (pixels)')
ax.set_ylabel('GPU time (ms, median)')
ax.set_title('Branch divergence vanishes once one stripe covers a whole subgroup\n'
             'Apple M1 Pro · 32 lanes · 420,000 fragments · 2000 loops',
             fontsize=9.5, loc='left', color=INK, pad=10)
for s in ('top', 'right'):
    ax.spines[s].set_visible(False)
ax.grid(axis='y', color='#ecebe7', lw=0.7)
ax.set_axisbelow(True)
fig.tight_layout()
fig.savefig('/home/claude/fig-stripes.png', dpi=220)

# ---------------- Figure 2: Part II cost cells ----------------
labels = ['1 triangle, small', '1 triangle, fullscreen', '100k instances, small']
vert = np.array([0.226, 0.238, 46.167])
frag = np.array([0.196, 1.645, 10.091])
ninv = [('3 vtx / 36 frag'), ('3 vtx / 360k frag'), ('300k vtx / 9.55M frag')]

fig, ax = plt.subplots(figsize=(6.3, 2.85))
y = np.arange(3)[::-1]
h = 0.33
ax.barh(y + h / 2 + 0.02, vert, height=h, color=BLUE, zorder=3)
ax.barh(y - h / 2 - 0.02, frag, height=h, color=ORANGE, zorder=3)
for yy, v in zip(y + h / 2 + 0.02, vert):
    ax.text(v * 1.12, yy, f'{v:.3f}', va='center', fontsize=7.8, color=INK)
for yy, v in zip(y - h / 2 - 0.02, frag):
    ax.text(v * 1.12, yy, f'{v:.3f}', va='center', fontsize=7.8, color=INK)

ax.set_yticks(y)
ax.set_yticklabels([f'{l}\n{n}' for l, n in zip(labels, ninv)], fontsize=8)
ax.set_xscale('log')
ax.set_xlim(0.1, 900)
ax.set_xlabel('GPU time above the loops = 0 baseline (ms, median, log scale)')
ax.set_title('The loop body is identical — only which stage runs it changes\n'
             'costLoops = 2000, baseline subtracted',
             fontsize=9.5, loc='left', color=INK, pad=10)
ax.legend([plt.Rectangle((0, 0), 1, 1, fc=BLUE), plt.Rectangle((0, 0), 1, 1, fc=ORANGE)],
          ['loop in the vertex stage', 'loop in the fragment stage'],
          loc='upper right', bbox_to_anchor=(1.0, 1.02), frameon=False,
          fontsize=8, labelcolor=INK2, handlelength=1.1, handleheight=0.9)
for s in ('top', 'right', 'left'):
    ax.spines[s].set_visible(False)
ax.tick_params(axis='y', length=0)
ax.grid(axis='x', color='#ecebe7', lw=0.7)
ax.set_axisbelow(True)
fig.tight_layout()
fig.savefig('/home/claude/fig-cost.png', dpi=220)
print('ok')
