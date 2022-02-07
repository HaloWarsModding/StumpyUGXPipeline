using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LSLib.Granny;
using LSLib.Granny.Model;
using LSLib.Granny.GR2;
using LSLib.LS;

namespace DAEtoGR2
{
    class Program
    {
        static void Main(string[] args)
        {
            Exporter e = new Exporter();
            e.Options.InputFormat = ExportFormat.DAE;
            e.Options.OutputFormat = ExportFormat.GR2;
            e.Options.Is64Bit = true;
            e.Options.RecalculateOBBs = true;
            e.Options.RecalculateNormals = false;
            e.Options.RecalculateTangents = false;
            e.Options.ConformAnimations = false;
            e.Options.IgnoreUVNaN = true;
            e.Options.DeduplicateUVs = true;
            e.Options.DeduplicateVertices = true;
            e.Options.RemoveTrivialAnimationKeys = false;

            GR2Utils.SaveModel(GR2Utils.LoadModel(args[1],
                new ExporterOptions { InputFormat = ExportFormat.DAE }), args[2], e);
            Root r = new Root();
            
            e.Options.Is64Bit = false;
            GR2Utils.SaveModel(GR2Utils.LoadModel(args[1],
                  new ExporterOptions { InputFormat = ExportFormat.DAE }), args[2] + ".32bit.gr2", e);

            Console.WriteLine("DAEGR2: done.");
        }
    }
}