# predictions.md — Part II, written before running `make cost`

Lab 06, CP479. Filled in from the geometry in `shaders/cost.vert` and the
`SCENES` table in `src/cost.cpp`, before any measurement.

## 1. What each scene actually draws

`cost.vert` places every instance with

```glsl
vec2 cell = vec2(gl_InstanceIndex % columns, gl_InstanceIndex / columns);
vec2 step = vec2(2.0) / float(columns);
vec2 pos  = inPosition * scale + cell * step - vec2(1.0);
```

so a scene is fully determined by `scale`, `instances` and `columns`. `TRIANGLE`
spans ±0.5, giving an unclipped clip-space area of 0.5 against a clip square of
area 4 (the 12.5% of Part I). The target is 800 × 600 = 480,000 pixels.

| Scene | scale | inst | cols | placement | unclipped area | visible | fragments | vertex inv. |
|---|---|---|---|---|---|---|---|---|
| S1 1 triangle, small | 0.04 | 1 | 1 | corner (−1,−1) | 0.5·0.04² = 8.0e−4 | 37.5% (rest is off-screen) | **36** | **3** |
| S2 1 triangle, fullscreen | 4.0 | 1 | 1 | corner (−1,−1), 4× | 8.0 | 75% of the square | **360,000** | **3** |
| S3 100k instances, small | 0.04 | 100,000 | 317 | 317 × 316 grid | 100,000 · 8.0e−4 = 80 | 100% covered, ≈20× overdraw | **≈9,551,000** | **300,000** |

How the fragment counts were obtained:

* **S2** is the only one the printed `rasterized` figure can give directly. Its
  three clipped vertices are (−1,−3), (−3,1), (1,1); the only edge that cuts the
  square is `y = 2x − 1`, and the excluded wedge has area ∫₀¹ 2x dx = 1 out of 4,
  so 75% → 0.75 × 480,000 = 360,000.
* **S1** sits on the corner, so only the quadrant `x ≥ −1, y ≥ −1` survives:
  ∫₀¹ (y+1)/2 dy / 2 = 0.375 of the triangle, i.e. 0.375 × 8.0e−4 / 4 × 480,000 = 36 px.
* **S3 cannot be read off `rasterized` at all.** Each triangle is 16 × 12 px but
  the grid pitch is only 2.52 × 1.89 px, so the triangles overlap about 20-fold
  and coverage saturates at 100%. The fragment count has to come from
  Σ(area) = 100,000 × 8.0e−4 = 80 clip units = 20 × the square = 9,600,000,
  less ≈0.5% lost where the grid runs off the right and top edges → ≈9,551,000.

(Confirmed by rasterizing all three scenes at 800 × 600 in a separate script:
36 / 360,000 / 9,551,435 fragments, coverage 0.007% / 75.000% / 100.000%.)

## 2. Predicted six cells

`LOOPS = {0, 500, 2000}`, and the loop body is the same instruction sequence in
both shaders, so the cost of a cell should be

> t ≈ fixed pass overhead + (invocations in the looping stage) × loops × c

with the same `c` in both stages. Predicted **iteration counts** at loops = 2000:

| Scene | vertex-stage loop | fragment-stage loop |
|---|---|---|
| S1 small | 3 × 2000 = 6.0e3 | 36 × 2000 = 7.2e4 |
| S2 fullscreen | 3 × 2000 = 6.0e3 | 360,000 × 2000 = 7.2e8 |
| S3 100k inst | 300,000 × 2000 = 6.0e8 | 9.551e6 × 2000 = 1.91e10 |

Predictions, in words, to be checked against the panel:

1. **S1 is noise in both columns.** 6e3 and 7.2e4 iterations are nothing; all
   six S1 numbers should be flat across loops = 0, 500, 2000 and sit at the
   timer's floor.
2. **S2 is flat in the vertex column, steep in the fragment column.** 3 vertices
   cannot be made expensive; 360,000 fragments can. Expect the fragment row to
   scale close to linearly from loops 500 → 2000 (a 4× rise).
3. **S3 is steep in both columns**, and this is where I expect to disagree with
   the handout's "isolates the vertex stage". S3 does have 100,000× more vertex
   work than S2, but the 20× overdraw means it also has 26.5× more *fragment*
   work than S2. So S3's fragment cell, not S3's vertex cell, should be the
   largest number on the panel.
4. **The two largest cells should be S3-fragment and S2-fragment, in a ratio of
   9.551e6 / 3.60e5 ≈ 26.5.** Second candidate pairing: S3-fragment vs
   S3-vertex, ratio 9.551e6 / 3.0e5 ≈ 31.8.
5. **At loops = 0** the six cells are pure rasterization, and they should still
   be ordered S1 ≪ S2 < S3, because 9.55M fragments and 300k vertices cost
   something even with an empty loop. This baseline is what must be subtracted
   before quoting any ratio.
6. **Resolution floor.** The smallest cell is bounded below by the timestamp
   period × query resolution reported by `./info` and by the variance of the
   21 measured frames; the honest answer for "smallest resolvable difference"
   is the spread between median and p95 on the cheapest cell, not the tick.

## 3. Predicted numbers

| Scene | stage | loops 0 | loops 500 | loops 2000 |
|---|---|---|---|---|
| S1 small | vertex | floor | floor | floor |
| S1 small | fragment | floor | floor | floor |
| S2 fullscreen | vertex | *b₂* | *b₂* | *b₂* |
| S2 fullscreen | fragment | *b₂* | *b₂* + Δ | *b₂* + 4Δ |
| S3 100k inst | vertex | *b₃* | *b₃* + 0.83Δ | *b₃* + 3.3Δ |
| S3 100k inst | fragment | *b₃* | *b₃* + 26.5Δ | *b₃* + 106Δ |

where Δ is the fragment-stage cost of 500 loops over 360,000 invocations on this
GPU, *b₂* and *b₃* the loops = 0 baselines. Absolute milliseconds are left to the
measurement; the ratios above are the prediction being tested.

## 4. Where I expect to be wrong

* A tile-based GPU (Apple silicon) may hide part of S3's overdraw, pushing the
  measured S3-fragment/S2-fragment ratio below 26.5.
* The compiler may keep the loop but vectorise it differently in the two stages,
  so `c` is not exactly equal vertex vs fragment; a ratio off by <2× is not
  evidence against the invocation-count model.
* At loops = 0 both shaders still evaluate the `clamp(x * 1e-9, ...)` tail, so
  *b₂*, *b₃* are not zero-work baselines.
