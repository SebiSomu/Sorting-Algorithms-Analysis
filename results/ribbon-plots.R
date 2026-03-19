# ─────────────────────────────────────────────────────────────────────────────
# Sorting Algorithms — Ribbon Plot Analysis
# Mean time across cases (random / sorted / reversed) with min/max shading
# ─────────────────────────────────────────────────────────────────────────────

library(ggplot2)
library(dplyr)
library(tidyr)

# ── 1. Load data ──────────────────────────────────────────────────────────────
# CSV is UTF-16 because it was generated via PowerShell pipe
df <- read.csv("sorting-results.csv", fileEncoding = "UTF-16LE", stringsAsFactors = FALSE)

# Quick check
cat("Rows loaded:", nrow(df), "\n")
cat("Algorithms :", paste(sort(unique(df$algorithm)), collapse = ", "), "\n")
cat("Cases      :", paste(sort(unique(df$case)), collapse = ", "), "\n")
cat("n range    :", min(df$n), "–", max(df$n), "\n\n")

# ── 2. Assign complexity class to each algorithm ──────────────────────────────
complexity_map <- c(
  "Swap Sort"         = "O(n²)",
  "Bubble Sort"       = "O(n²)",
  "Selection Sort"    = "O(n²)",
  "Insertion Sort"    = "O(n²)",
  "Cocktail Sort"     = "O(n²)",
  "Gnome Sort"        = "O(n²)",
  "Merge Sort"        = "O(n log n)",
  "Heap Sort"         = "O(n log n)",
  "Intro Sort"        = "O(n log n)",
  "Tim Sort"          = "O(n log n)",
  "Quick Sort (Rec)"  = "O(n log n)",
  "Quick Sort (Iter)" = "O(n log n)",
  "Tree Sort"         = "O(n log n)",
  "Comb Sort"         = "O(n log n)",
  "Shell Sort"        = "O(n log² n)",
  "Bitonic Sort"      = "O(n log² n)",
  "Count Sort"        = "O(n)",
  "Radix Sort"        = "O(n)",
  "Pigeonhole Sort"   = "O(n)",
  "Bucket Sort"       = "O(n)"
)

df$complexity <- complexity_map[df$algorithm]

# ── 3. Compute mean / min / max across cases per (algorithm, n) ───────────────
# Quick Sort (Rec) has only random — excluded from ribbon (only 1 case = no spread)
df_stats <- df %>%
  group_by(algorithm, complexity, n) %>%
  summarise(
    mean_time  = mean(time_ms),
    min_time   = min(time_ms),
    max_time   = max(time_ms),
    n_cases    = n(),
    .groups    = "drop"
  ) %>%
  filter(n_cases >= 2)   # need at least 2 cases for a meaningful ribbon

cat("Algorithms included in ribbon plots:\n")
print(sort(unique(df_stats$algorithm)))

# ── 4. Color palette (one color per algorithm) ────────────────────────────────
algo_colors <- c(
  "Swap Sort"         = "#F44336",
  "Bubble Sort"       = "#E91E63",
  "Selection Sort"    = "#9C27B0",
  "Insertion Sort"    = "#673AB7",
  "Cocktail Sort"     = "#FF5722",
  "Gnome Sort"        = "#FF9800",
  "Merge Sort"        = "#2196F3",
  "Heap Sort"         = "#03A9F4",
  "Intro Sort"        = "#00BCD4",
  "Tim Sort"          = "#009688",
  "Quick Sort (Iter)" = "#4CAF50",
  "Tree Sort"         = "#8BC34A",
  "Comb Sort"         = "#CDDC39",
  "Shell Sort"        = "#FFC107",
  "Bitonic Sort"      = "#FF9800",
  "Count Sort"        = "#795548",
  "Radix Sort"        = "#607D8B",
  "Pigeonhole Sort"   = "#9E9E9E",
  "Bucket Sort"       = "#455A64"
)

# ── 5. Shared theme ───────────────────────────────────────────────────────────
theme_sorting <- function() {
  theme_minimal(base_size = 11) +
    theme(
      plot.background    = element_rect(fill = "#0d0d0d", color = NA),
      panel.background   = element_rect(fill = "#0d0d0d", color = NA),
      panel.grid.major   = element_line(color = "#222222"),
      panel.grid.minor   = element_line(color = "#1a1a1a"),
      strip.background   = element_rect(fill = "#1a1a1a", color = NA),
      strip.text         = element_text(color = "white", face = "bold", size = 10),
      axis.text          = element_text(color = "#aaaaaa"),
      axis.title         = element_text(color = "white"),
      plot.title         = element_text(color = "white", face = "bold", size = 14),
      plot.subtitle      = element_text(color = "#aaaaaa", size = 10),
      legend.background  = element_rect(fill = "#0d0d0d", color = NA),
      legend.text        = element_text(color = "#cccccc"),
      legend.title       = element_text(color = "white"),
      legend.key         = element_rect(fill = "#0d0d0d", color = NA)
    )
}

fmt_n <- function(x) {
  ifelse(x >= 1000, paste0(x / 1000, "k"), as.character(x))
}

# ── 6. Plot: all complexity classes in one figure (faceted) ───────────────────
# Order complexity classes from fastest to slowest
df_stats$complexity <- factor(df_stats$complexity,
  levels = c("O(n)", "O(n log n)", "O(n log² n)", "O(n²)"))

p_all <- ggplot(df_stats, aes(x = n, color = algorithm, fill = algorithm)) +
  geom_ribbon(aes(ymin = min_time, ymax = max_time), alpha = 0.15, color = NA) +
  geom_line(aes(y = mean_time), linewidth = 1.1) +
  facet_wrap(~complexity, scales = "free_y", ncol = 2) +
  scale_color_manual(values = algo_colors, name = "Algorithm") +
  scale_fill_manual(values = algo_colors, name = "Algorithm") +
  scale_x_continuous(labels = fmt_n) +
  scale_y_continuous(labels = scales::comma) +
  labs(
    title    = "Sorting Algorithms — Mean Time with Input Sensitivity",
    subtitle = "Line = mean across cases  |  Shaded area = spread between min and max case",
    x        = "n  (input size)",
    y        = "time_ms"
  ) +
  theme_sorting()

print(p_all)
ggsave("ribbon_all_classes.png", p_all, width = 14, height = 10, dpi = 130,
       bg = "#0d0d0d")
cat("Saved: ribbon_all_classes.png\n")

# ── 7. Individual plots per complexity class ───────────────────────────────────
complexity_classes <- levels(df_stats$complexity)

for (cls in complexity_classes) {
  df_cls <- df_stats %>% filter(complexity == cls)

  p <- ggplot(df_cls, aes(x = n, color = algorithm, fill = algorithm)) +
    geom_ribbon(aes(ymin = min_time, ymax = max_time), alpha = 0.2, color = NA) +
    geom_line(aes(y = mean_time), linewidth = 1.3) +
    scale_color_manual(values = algo_colors, name = "Algorithm") +
    scale_fill_manual(values = algo_colors, name = "Algorithm") +
    scale_x_continuous(labels = fmt_n) +
    scale_y_continuous(labels = scales::comma) +
    labs(
      title    = paste0(cls, " Algorithms — Mean Time & Input Sensitivity"),
      subtitle = "Line = mean(random, sorted, reversed)  |  Shaded = min/max spread",
      x        = "n  (input size)",
      y        = "time_ms"
    ) +
    theme_sorting()

  print(p)

  # Clean filename: remove parentheses and spaces
  fname <- paste0("ribbon_", gsub("[^a-zA-Z0-9]", "_", cls), ".png")
  ggsave(fname, p, width = 12, height = 6, dpi = 130, bg = "#0d0d0d")
  cat("Saved:", fname, "\n")
}

# ── 8. Bonus: sensitivity plot ────────────────────────────────────────────────
# Shows how much each algorithm's time varies depending on input type
# Metric: (max_time - min_time) / mean_time  at the largest n available per algo

df_sensitivity <- df_stats %>%
  group_by(algorithm, complexity) %>%
  filter(n == max(n)) %>%
  mutate(sensitivity = (max_time - min_time) / mean_time) %>%
  ungroup() %>%
  arrange(desc(sensitivity))

df_sensitivity$algorithm <- factor(df_sensitivity$algorithm,
  levels = df_sensitivity$algorithm)

p_sens <- ggplot(df_sensitivity,
                 aes(x = algorithm, y = sensitivity, fill = complexity)) +
  geom_col(alpha = 0.85) +
  geom_text(aes(label = sprintf("%.2f", sensitivity)),
            hjust = -0.15, color = "white", size = 3.2) +
  coord_flip() +
  scale_fill_manual(values = c(
    "O(n)"       = "#607D8B",
    "O(n log n)" = "#2196F3",
    "O(n log² n)"= "#FF9800",
    "O(n²)"      = "#F44336"
  ), name = "Complexity") +
  scale_y_continuous(expand = expansion(mult = c(0, 0.15))) +
  labs(
    title    = "Input Sensitivity per Algorithm",
    subtitle = "(max_time − min_time) / mean_time  at largest measured n",
    x        = NULL,
    y        = "Sensitivity ratio"
  ) +
  theme_sorting() +
  theme(panel.grid.major.y = element_blank())

print(p_sens)
ggsave("ribbon_sensitivity.png", p_sens, width = 10, height = 8, dpi = 130,
       bg = "#0d0d0d")
cat("Saved: ribbon_sensitivity.png\n\n")
cat("Done! Files generated:\n")
cat("  ribbon_all_classes.png\n")
for (cls in complexity_classes) {
  cat("  ribbon_", gsub("[^a-zA-Z0-9]", "_", cls), ".png\n", sep = "")
}
cat("  ribbon_sensitivity.png\n")