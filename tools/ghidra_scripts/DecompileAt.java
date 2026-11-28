// Headless helper: print program stats and the decompiled C of the functions at the given addresses.
// usage: analyzeHeadless <proj> <name> -process <file> -noanalysis -scriptPath <dir> -postScript DecompileAt.java 0x340B9B ...
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;

public class DecompileAt extends GhidraScript {
    @Override
    public void run() throws Exception {
        println("functions: " + currentProgram.getFunctionManager().getFunctionCount()
            + ", blocks: " + currentProgram.getMemory().getBlocks().length
            + ", compiler spec: " + currentProgram.getCompilerSpec().getCompilerSpecID());
        DecompInterface dec = new DecompInterface();
        dec.openProgram(currentProgram);
        int maxLines = Integer.MAX_VALUE;  // optional first argument "lines:N" (not "=": the .bat launcher splits arguments at "="): print at most N lines per function
        for (String arg : getScriptArgs()) {
            if (arg.startsWith("lines:")) { maxLines = Integer.parseInt(arg.substring(6)); continue; }
            Address a = toAddr(arg);
            Function f = getFunctionContaining(a);
            if (f == null) { println(arg + ": no function"); continue; }
            DecompileResults r = dec.decompileFunction(f, 60, monitor);
            String c = r.decompileCompleted() ? r.getDecompiledFunction().getC() : "(decompile failed: " + r.getErrorMessage() + ")";
            String[] ls = c.split("\n");
            if (ls.length > maxLines) c = String.join("\n", java.util.Arrays.copyOf(ls, maxLines)) + "\n  ... (" + (ls.length - maxLines) + " more lines)";
            println("==== " + arg + " in " + f.getName() + " @ " + f.getEntryPoint() + " (" + f.getCallingConventionName() + ")\n" + c);
        }
        dec.dispose();
    }
}
