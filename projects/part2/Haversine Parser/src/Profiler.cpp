struct profile_anchor
{
    u64 ElapsedTSC_Exclusive; // Excludes children
    u64 ElapsedTSC_Inclusive; // Includes children

    u64 HitCount; // Number of times a profile_block has been entered and exited during the run of a program

    char const* Label;
};

struct profiler
{
    profile_anchor Anchors[4096];

    u64 StartTSC;
    u64 EndTSC;
};
static profiler GlobalProfiler;
static u32 GlobalProfiler_ParentIndex;

struct profile_block
{
    char const* Label;
    u64 StartTSC;
    u64 PreviousElapsedTSC_Inclusive;
    u32 ParentIndex;
    u32 AnchorIndex;

    profile_block( char const* label, u32 anchorIndex )
    {
        ParentIndex = GlobalProfiler_ParentIndex;

        AnchorIndex = anchorIndex;
        Label = label;

        profile_anchor* Anchor = GlobalProfiler.Anchors + AnchorIndex;
        PreviousElapsedTSC_Inclusive = Anchor->ElapsedTSC_Inclusive;

        GlobalProfiler_ParentIndex = AnchorIndex;
        StartTSC = ReadCPUTimer();
    }

    ~profile_block()
    {
        u64 Elapsed = ReadCPUTimer() - StartTSC;
        GlobalProfiler_ParentIndex = ParentIndex;

        profile_anchor* Parent = GlobalProfiler.Anchors + ParentIndex;
        Parent->ElapsedTSC_Exclusive -= Elapsed;

        profile_anchor* Anchor = GlobalProfiler.Anchors + AnchorIndex;
        Anchor->ElapsedTSC_Exclusive += Elapsed;
        Anchor->ElapsedTSC_Inclusive = PreviousElapsedTSC_Inclusive + Elapsed;

        ++Anchor->HitCount;
        Anchor->Label = Label;
    }
};

// The TimeFunction macro is just a way to call the TimeBlock macro automatically with the name of the containing function. 
// And NameConcat/NameConcat2 are just workarounds for the fact that C++’s macro preprocessor is terrible, 
// and can’t merge the value of a preprocessor macro with an identifier any other way. 
// So really, there is only one macro here worth noting, and that’s TimeBlock.

// TimeBlock creates a profile_block structure on the stack with a unique name: “Block” plus the line number on which the macro occurs. 
// This “uniquing” of the name is just to make sure that no two profile blocks in the same function end up with the same name, 
// leading to “name shadowing” compiler warnings.

// For the profile_block label, TimeBlock passes whatever string it was given. 
// This is either the function name, in the case of TimeFunction, or an explicit name if the user used the TimeBlock macro directly.

// For the AnchorIndex, TimeBlock uses C++’s built-in __COUNTER__ macro, which is guaranteed to evaluate to a unique index every time. 
// TimeBlock passes this value, incremented by one, as the AnchorIndex. 
// This ensures that a) every profile block will have a unique anchor, and b) anchor 0 will never be used.
#define _NameConcat(A, B) A##B
#define NameConcat(A, B) _NameConcat( A, B )
#define TimeBlock(Name) profile_block NameConcat( Block, __LINE__ )( Name, __COUNTER__ + 1 )
#define TimeFunction TimeBlock( __func__ )

static void PrintTimeElapsed( u64 totalElapsedTSC, profile_anchor* anchor )
{
    f64 Percent = 100.0 * ((f64)anchor->ElapsedTSC_Exclusive / (f64)totalElapsedTSC);
    WRITE( "  %s[%llu]: %llu (%.2f%%", anchor->Label, anchor->HitCount, anchor->ElapsedTSC_Exclusive, Percent );

    if( anchor->ElapsedTSC_Inclusive != anchor->ElapsedTSC_Exclusive )
    {
        f64 PercentWithChildren = 100.0 * ((f64)anchor->ElapsedTSC_Inclusive / (f64)totalElapsedTSC);
        WRITE( ", %.2f%% w/children", PercentWithChildren );
    }
    WRITE( ")\n" );
}

static void BeginProfile( void )
{
    GlobalProfiler.StartTSC = ReadCPUTimer();
}

static void EndProfileAndPrint()
{
    GlobalProfiler.EndTSC = ReadCPUTimer();
    u64 CPUFreq = EstimeCPUFrequency();

    u64 TotalCPUElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;

    if( CPUFreq )
    {
        WRITE_LINE( "Total time: %0.4fms (CPU freq %llu)", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq );
    }

    for( u32 AnchorIndex = 0; AnchorIndex < ArrayCount( GlobalProfiler.Anchors ); ++AnchorIndex )
    {
        profile_anchor* Anchor = GlobalProfiler.Anchors + AnchorIndex;
        if( Anchor->ElapsedTSC_Inclusive )
        {
            PrintTimeElapsed( TotalCPUElapsed, Anchor );
        }
    }
}