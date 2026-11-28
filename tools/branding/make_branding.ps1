<#
Make the installer/branding images Heroes Recompiled embeds, at the sizes and layouts the code expects.

  make_branding.ps1 -Out ShadowRecompResources                     # public placeholders (no inputs)
  make_branding.ps1 -Out ShadowRecompResources\local -Background <img> -Logo <img> [-Music <audio>]

Outputs (same paths as in the resources repo; -Out is the repo root or its local\ override folder):
  images\installer\installer_bg.png   Any shape, longer side <= 1024. The installer fills the window with it,
                                      keeping its proportions and trimming the overflow (installer_wizard.cpp).
  images\installer\FoxLogo.png        640x512 RGBA. Drawn from the crop (71,17)-(591,499) next to the title text.
  images\game_icon.png/.bmp/.ico      128x128 window/exe icon.
  music\installer.ogg                 only with -Music (converted with ffmpeg, Ogg Vorbis: the mixer has no MP3).

Policy (docs/notes.md, "Installer branding"): like Shadow Recompiled, official game art may go in the public
resources; original game music may not. Put it in the gitignored local\ folder (-Out ...\local -Music ...).
Without inputs the script draws neutral placeholders.
#>
param(
    [Parameter(Mandatory)] [string] $Out,
    [string] $Background,
    [string] $Logo,
    [string] $Music
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
# .NET resolves relative paths against the process directory, not PowerShell's location: make it absolute.
New-Item -ItemType Directory -Force $Out | Out-Null
$Out = (Resolve-Path $Out).Path

function New-Canvas([int] $w, [int] $h) {
    $b = New-Object System.Drawing.Bitmap $w, $h, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($b)
    $g.SmoothingMode = 'AntiAlias'; $g.InterpolationMode = 'HighQualityBicubic'; $g.PixelOffsetMode = 'HighQuality'
    $g.Clear([System.Drawing.Color]::Transparent)
    return $b, $g
}
function Save-Png($bmp, [string] $path) {
    New-Item -ItemType Directory -Force (Split-Path $path) | Out-Null
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
}
# Fit $img inside the rectangle (x, y, w, h), keeping its aspect ratio, centred.
function Draw-Fit($g, $img, [float] $x, [float] $y, [float] $w, [float] $h) {
    $s = [Math]::Min($w / $img.Width, $h / $img.Height)
    $dw = $img.Width * $s; $dh = $img.Height * $s
    $g.DrawImage($img, [float]($x + ($w - $dw) / 2), [float]($y + ($h - $dh) / 2), [float]$dw, [float]$dh)
}
function Draw-Circles($g, [float] $cx, [float] $cy, [float] $r) {
    # Placeholder mark: three separate plain circles (red / blue / yellow), spaced so they never overlap
    # (overlapping rings would resemble the Olympic symbol). Not SEGA's emblem.
    $cols = @([System.Drawing.Color]::FromArgb(255, 220, 50, 40), [System.Drawing.Color]::FromArgb(255, 40, 110, 230), [System.Drawing.Color]::FromArgb(255, 245, 190, 30))
    $pos = @(@(-0.55, -0.35), @(0.55, -0.35), @(0, 0.6))
    for ($i = 0; $i -lt 3; $i++) {
        $px = $cx + $pos[$i][0] * $r * 2.4; $py = $cy + $pos[$i][1] * $r * 2.4
        $pen = New-Object System.Drawing.Pen $cols[$i], ($r * 0.32)
        $g.DrawEllipse($pen, [float]($px - $r), [float]($py - $r), [float](2 * $r), [float](2 * $r))
        $pen.Dispose()
    }
}

# ---- installer background: kept at its own proportions (the installer fills the window and trims the
# overflow), downscaled so the longer side is at most 1024 px.
if ($Background) {
    $img = [System.Drawing.Image]::FromFile((Resolve-Path $Background))
    $s = [Math]::Min(1.0, 1024.0 / [Math]::Max($img.Width, $img.Height))
    $bmp, $g = New-Canvas ([int][Math]::Round($img.Width * $s)) ([int][Math]::Round($img.Height * $s))
    $g.DrawImage($img, 0, 0, $bmp.Width, $bmp.Height)
    $img.Dispose()
} else {
    $bmp, $g = New-Canvas 1024 576
    $rect = New-Object System.Drawing.Rectangle 0, 0, 1024, 576
    $brush = New-Object System.Drawing.Drawing2D.LinearGradientBrush $rect, ([System.Drawing.Color]::FromArgb(255, 20, 60, 140)), ([System.Drawing.Color]::FromArgb(255, 5, 10, 30)), 90
    $g.FillRectangle($brush, $rect); $brush.Dispose()
}
Save-Png $bmp (Join-Path $Out 'images\installer\installer_bg.png'); $g.Dispose(); $bmp.Dispose()

# ---- logo (640x512, content inside the drawn crop 71,17 - 591,499)
$bmp, $g = New-Canvas 640 512
if ($Logo) {
    $img = [System.Drawing.Image]::FromFile((Resolve-Path $Logo))
    Draw-Fit $g $img 71 17 520 482
    $img.Dispose()
} else {
    Draw-Circles $g 331 258 72
}
Save-Png $bmp (Join-Path $Out 'images\installer\FoxLogo.png'); $g.Dispose(); $bmp.Dispose()

# ---- icon (128x128): png, 32-bit bmp, and an ico holding the png
$bmp, $g = New-Canvas 128 128
if ($Logo) {
    $img = [System.Drawing.Image]::FromFile((Resolve-Path $Logo))
    Draw-Fit $g $img 4 4 120 120
    $img.Dispose()
} else {
    $brush = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 15, 30, 70))
    $g.FillEllipse($brush, 2, 2, 124, 124); $brush.Dispose()
    Draw-Circles $g 64 64 14
}
$png = Join-Path $Out 'images\game_icon.png'
Save-Png $bmp $png
$bmp.Save((Join-Path $Out 'images\game_icon.bmp'), [System.Drawing.Imaging.ImageFormat]::Bmp)
$g.Dispose(); $bmp.Dispose()
# ICO with one 128x128 PNG entry (ICONDIR + ICONDIRENTRY + PNG data)
$pngBytes = [System.IO.File]::ReadAllBytes($png)
$ms = New-Object System.IO.MemoryStream
$w = New-Object System.IO.BinaryWriter $ms
$w.Write([uint16]0); $w.Write([uint16]1); $w.Write([uint16]1)                 # reserved, type=icon, count
$w.Write([byte]128); $w.Write([byte]128); $w.Write([byte]0); $w.Write([byte]0) # width, height, colours, reserved
$w.Write([uint16]1); $w.Write([uint16]32); $w.Write([uint32]$pngBytes.Length); $w.Write([uint32]22)
$w.Write($pngBytes); $w.Flush()
[System.IO.File]::WriteAllBytes((Join-Path $Out 'images\game_icon.ico'), $ms.ToArray())

# ---- music (optional)
if ($Music) {
    $dst = Join-Path $Out 'music\installer.ogg'
    New-Item -ItemType Directory -Force (Split-Path $dst) | Out-Null
    & ffmpeg -hide_banner -loglevel error -y -i $Music -vn -c:a libvorbis -q:a 5 $dst
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed" }
}
Write-Output "branding written to $Out"
