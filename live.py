import numpy as np
from scipy import signal
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider


class Equalizer:
    Fs = 48000
    GainMin = -30
    GainMax = 30
    Qmin = 0.001
    Qmax = 3.000

    def __init__(self, freq, gain, q):
        self.Q = q
        self.Freq = freq
        self.Gain_dB = gain
        self.Z = [0.0, 0.0]
        self.peaking_coef()

    def peaking_coef(self):
        # Peaking EQ coefficients
        A = 10 ** (self.Gain_dB / 40.0)
        w0 = 2 * np.pi * self.Freq / Equalizer.Fs
        cosw0 = np.cos(w0)
        sinw0 = np.sin(w0)
        alpha = sinw0 / (2 * self.Q)
        b0 = 1 + alpha * A
        b1 = -2 * cosw0
        b2 = 1 - alpha * A
        a0 = 1 + alpha / A
        a1 = -2 * cosw0
        a2 = 1 - alpha / A
        self.B = np.array([b0/a0, b1/a0, b2/a0])
        self.A = np.array([1, a1/a0, a2/a0])

    def biquad_response(self, w_rad):
        a = self.A
        b = self.B
        # Evaluate H(e^jw) = (b0 + b1 z^-1 + b2 z^-2) / (1 + a1 z^-1 + a2 z^-2)
        # where z = e^(jw)
        z = np.exp(1j * w_rad)
        num = b[0] + b[1]*z + b[2]*z**2
        den = 1 + a[1]*z + a[2]*z**2
        return num / den


    def biquad_tdf2(self, x):
        y = self.B[0] * x + self.Z[0]
        self.Z[0] = self.B[1] * x - self.A[1] * y + self.Z[1]
        self.Z[1] = self.B[2] * x - self.A[2] * y
        return y

    def updateGain(self, val):
        self.Gain_dB = val
        self.peaking_coef()
        Equalizer.replot()

    def updateQ(self, val):
        self.Q = val
        self.peaking_coef()
        Equalizer.replot()

    # Compute combined response
    w = np.linspace(20, 20000, 4096)

    @staticmethod
    def total_response(fs, bands):
        w_rad = 2 * np.pi * Equalizer.w / fs
        h_total = np.ones_like(w_rad, dtype=complex)
        for eq in bands:
            h = eq.biquad_response(w_rad)
            h_total *= h
        return Equalizer.w, h_total

    @staticmethod
    def plot(bands):
        # Initial state
        w, h = Equalizer.total_response(Equalizer.Fs, bands)

        # Plot setup
        Equalizer.fig, ax = plt.subplots(figsize=(10,6))
        plt.subplots_adjust(left=0.1, bottom=0.35)
        Equalizer.line, = ax.semilogx(w, 20*np.log10(np.abs(h)), lw=2)
        ax.set_xlim(20, 20000)
        ax.set_ylim(1.5*Equalizer.GainMin, 1.5*Equalizer.GainMax)
        ax.set_title("10-Band Peaking EQ")
        ax.set_xlabel("Frequency [Hz]")
        ax.set_ylabel("Magnitude [dB]")
        ax.grid(True, which='both', ls=':')

    @staticmethod
    def replot():
        # Update function
        w, h = Equalizer.total_response(Equalizer.Fs, eqs)
        Equalizer.line.set_ydata(20 * np.log10(np.abs(h)))
        Equalizer.fig.canvas.draw_idle()


eqs = [
    Equalizer(freq=31.25,  gain=0, q=0.707),
    Equalizer(freq=62.5,   gain=0, q=0.707),
    Equalizer(freq=125,    gain=0, q=0.707),
    Equalizer(freq=250,    gain=0, q=0.707),
    Equalizer(freq=500,    gain=0, q=0.707),
    Equalizer(freq=1000,   gain=0, q=0.707),
    Equalizer(freq=2000,   gain=0, q=0.707),
    Equalizer(freq=4000,   gain=0, q=0.707),
    Equalizer(freq=8000,   gain=0, q=0.707),
    Equalizer(freq=16000,  gain=0, q=0.707),
]
Equalizer.plot(eqs)


# Each row for one band
for i, eq in enumerate(eqs):
    y = 0.05 + (i * 0.02)
    # Gain slider (left)
    ax_gain = plt.axes([0.15, y, 0.35, 0.015])
    eq.SliderGain = Slider(ax_gain, f"{eq.Freq:>6.1f} Hz G", Equalizer.GainMin, Equalizer.GainMax, valinit=eq.Gain_dB)
    eq.SliderGain.on_changed(eq.updateGain)

    # # Q slider (right)
    ax_q = plt.axes([0.55, y, 0.35, 0.015])
    eq.SliderQ = Slider(ax_q, f"Q", Equalizer.Qmin, Equalizer.Qmax, valinit=eq.Q)
    eq.SliderQ.on_changed(eq.updateQ)

## Live sound input/output

import sounddevice as sd
import numpy as np
import threading

fs = 48000
blocksize = 2048

y = np.zeros(blocksize)

def process_sample(x):
    """Process one sample through all filters"""
    y = x
    for band in eqs:
        y = band.biquad_tdf2(y)
    return y

def process_block(x):
    """Process a full array"""
    for i, s in enumerate(x):
        y[i] = process_sample(s)
    return y

def audio_thread():
    def callback(indata, outdata, frames, time, status):
        if status:
            print(status)
        outdata[:] = process_block(indata[:, 0]).reshape(-1, 1)
    with sd.Stream(channels=1, dtype='float32', samplerate=fs, blocksize=blocksize, callback=callback):
        print("Streaming started... (Ctrl+C to stop)")
        threading.Event().wait()  # keep alive

# Start audio stream in background


if __name__ == "__main__":
    thread = threading.Thread(target=audio_thread, daemon=True)
    thread.start()
    plt.show()