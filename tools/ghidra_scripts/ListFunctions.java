// Headless helper: write every function Ghidra knows as "0xENTRY 0xSIZE name" (size = entry to end of its
// contiguous body), one per line, to the file given as the first script argument.
// usage: analyzeHeadless <proj> <name> -process <file> -noanalysis -readOnly -scriptPath tools/ghidra_scripts
//        -postScript ListFunctions.java D:/path/out.txt
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import java.io.PrintWriter;

public class ListFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String out = getScriptArgs()[0];
        int n = 0;
        try (PrintWriter w = new PrintWriter(out)) {
            for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
                Address entry = f.getEntryPoint();
                AddressRange first = f.getBody().getRangeContaining(entry);
                long size = first == null ? 0 : first.getMaxAddress().subtract(entry) + 1;
                w.printf("0x%X 0x%X %s%n", entry.getOffset(), size, f.getName());
                n++;
            }
        }
        println("wrote " + n + " functions to " + out);
    }
}
