using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LSLib.Granny;
using LSLib.Granny.Model;
using LSLib.Granny.GR2;
using LSLib.LS;
using Assimp;
using Animation = Assimp.Animation;
using Mesh = Assimp.Mesh;

namespace DAEtoGR2
{
    class Program
    {
        static void Main(string[] args)
        {
            //AssimpContext assimpContext = new AssimpContext();
            //Scene scene = assimpContext.ImportFile(args[1]);

            //if(!scene.HasAnimations)
            //{
            //    Console.WriteLine("No animation.");
            //    return;
            //}

            //for(int i = 0; i < scene.AnimationCount; i++)
            //{
            //    for(int j = 0; j < scene.Animations[i].NodeAnimationChannelCount; j++)
            //    {
            //        //Console.WriteLine(scene.Animations[i].NodeAnimationChannels[j].NodeName);
            //    }
            //}
            //foreach(Mesh m in scene.Meshes)
            //{
            //    //m.
            //}
            //assimpContext.ExportFile(scene, args[1] + ".dae", assimpContext.GetSupportedExportFormats()[0].FormatId);
            

            Exporter e = new Exporter();
            e.Options.InputFormat = ExportFormat.DAE;
            e.Options.OutputFormat = ExportFormat.GR2;
            e.Options.Is64Bit = true;
            e.Options.RecalculateOBBs = false;
            e.Options.RecalculateNormals = false;
            e.Options.RecalculateTangents = false;
            e.Options.ConformAnimations = false;
            e.Options.IgnoreUVNaN = true;
            e.Options.DeduplicateUVs = true;
            e.Options.DeduplicateVertices = true;
            e.Options.RemoveTrivialAnimationKeys = false;

            GR2Utils.SaveModel(GR2Utils.LoadModel(args[1],
                new ExporterOptions { InputFormat = ExportFormat.DAE }), args[2], e);

            if (args[0] == "-debug")
            {
                e.Options.Is64Bit = false;
                GR2Utils.SaveModel(GR2Utils.LoadModel(args[1],
                      new ExporterOptions { InputFormat = ExportFormat.DAE }), args[2] + ".32bit.gr2", e);
            }

            Console.WriteLine("FINISHED");

            //Console.ReadLine();
        }
    }
}