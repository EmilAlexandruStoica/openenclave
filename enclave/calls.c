#include <openenclave/enclave.h>
#include <openenclave/bits/jump.h>
#include <openenclave/bits/sgxtypes.h>
#include <openenclave/bits/fault.h>
#include <openenclave/bits/calls.h>
#include <openenclave/bits/reloc.h>
#include <openenclave/bits/globals.h>
#include "asmdefs.h"
#include "td.h"
#include "init.h"

typedef unsigned long long WORD;

#define WORD_SIZE sizeof(WORD)

/*
**==============================================================================
**
** Glossary:
**
**     TCS      - Thread control structure. The TCS is an address passed to
**                EENTER and passed onto the entry point (OE_Main). The TCS
**                is the address of a TCS page in the enclave memory. This page
**                is not accessible to the enclave itself. The enclave stores
**                state about the execution of a thread in this structure,
**                such as the entry point (TCS.oentry), which refers to the
**                OE_Main function. It also maintains the index of the 
**                current SSA (TCS.cssa) and the number of SSA's (TCS.nssa).
**
**     TD       - Thread data. Per thread data as defined by the OE_ThreadData
**                structure and extended by the TD structure. This structure
**                records the stack pointer of the last EENTER.
**
**     SP       - Stack pointer. Refers to the enclave's stack pointer.
**
**     BP       - Base pointer. Refers to the enclave's base pointer.
**
**     HOSTSP   - Host stack pointer. Refers to the host's stack pointer as
**                received in the EENTER call.
**
**     HOSTBP   - Host base pointer. Refers to the host's base pointer as
**                received in the EENTER call.
**
**     AEP      - Asynchronous Exception Procedure. This procedure is passed
**                by the host to EENTER. If a fault occurs while in the enclave,
**                the hardware calls this procedure. The procedure may
**                terminate or call ERESUME to continue executing in the
**                enclave.
**
**     AEX      - Asynchronous Exception (occurs when enclave faults). The
**                hardware transfers control to a host AEP (passed as a
**                parameter to EENTER).
**
**     SSA      - State Save Area. When a fault occurs in the enclave, the
**                hardware saves the state here (general purpose registers) 
**                and then transfers control to the host AEP. If the AEP
**                executes the ERESUME instruction, the hardware restores the
**                state from the SSA.
**
**     EENTER   - An untrusted instruction that is executed by the host to
**                enter the enclave. The caller passes the address of a TCS page
**                within the enclave, an AEP, and any parameters in the RDI and 
**                RSI registers. This implementation passes the operation 
**                number (FUNC) in RDI and a pointer to the arguments structure 
**                (ARGS) in RSI.
**
**     EEXIT    - An instruction that is executed by the host to exit the
**                enclave and return control to the host. The caller passes
**                the address of some instruction to jump to (RETADDR) in the
**                RBX register and an AEP in the RCX register (null at this
**                time).
**
**     RETADDR  - Refers to the address of the return instruction that the
**                hardware jumps to from EEXIT. This is an instruction in
**                host immediately following the instruction that executed
**                EENTER.
**
**     CSSA     - The current SSA slot index (as given by TCS.cssa). EENTER
**                passes a CSSA parameter (RAX) to OE_Main(). A CSSA of zero
**                indicates a normal entry. A non-zero CSSA indicates an 
**                exception entry (an AEX has occurred).
**
**     NSSA     - The number of SSA slots in the thread section (of this
**                enclave. If CSSA == NSSA, then the SSA's have been exhausted
**                and the EENTER instruction will fault.
**
**     ECALL    - A function call initiated by the host and carried out by
**                the enclave. The host executes the EENTER instruction to
**                enter the enclave.
**
**     ERET     - A return from an ECALL initiated by the enclave. The 
**                enclave executes the EEXIT instruction to exit the enclave.
**
**     OCALL    - A function call initiated by the enclave and carried out
**                by the host. The enclave executes the EEXIT instruction to
**                exit the enclave.
**
**     ORET     - A return from an OCALL initiated by the host. The host
**                executes the EENTER instruction to enter the enclave.
**
**==============================================================================
*/

/*
**==============================================================================
**
** _HandleCallEnclave()
**
**     This function handles a high-level enclave call.
**
**==============================================================================
*/

static void _HandleCallEnclave(oe_uint64_t argIn, oe_uint64_t* argOut)
{
    OE_CallEnclaveArgs* args = (OE_CallEnclaveArgs*)argIn;

    if (!args->func)
    {
        args->result = OE_INVALID_PARAMETER;
        return;
    }

#if 0
    if (!OE_IsWithinEnclave(args->func, 1))
    {
        args->result = OE_FAILURE;
        return;
    }
#endif

    /* ATTN: check that address is within text segment */

    args->func(args->args);
    args->result = OE_OK;
    
    if (argOut)
        *argOut = 0;
}

/*
**==============================================================================
**
** _HandleExit()
**
**     Initiate call to EEXIT.
**
**==============================================================================
*/

static void _HandleExit(
    OE_Code code,
    long func, 
    oe_uint64_t arg)
{
    OE_Exit(OE_MAKE_WORD(code, func), arg);
}

/*
**==============================================================================
**
** _HandleECALL()
**
**     Handle an ECALL.
**
**==============================================================================
*/

static void _HandleECALL(
    TD* td,
    int func,
    oe_uint64_t argIn)
{
    /* Insert ECALL context onto front of TD.ecalls list */
    Callsite callsite;
    oe_uint64_t argOut = 0;

    OE_Memset(&callsite, 0, sizeof(callsite));
    TD_PushCallsite(td, &callsite);

    /* Dispatch the ECALL */
    switch (func)
    {
        case OE_FUNC_CALL_ENCLAVE:
        {
            _HandleCallEnclave(argIn, &argOut);
            break;
        }
        default:
        {
            /* ATTN: what action should be taken here? */
            OE_Abort();
            break;
        }
    }

    /* Remove ECALL context from front of TD.ecalls list */
    TD_PopCallsite(td);

    /* Perform ERET, giving control back to host */
    _HandleExit(OE_CODE_ERET, func, argOut);
}

/*
**==============================================================================
**
** _HandleORET()
**
**     Handle an OCALL return.
**
**==============================================================================
*/

static __inline__ void _HandleORET(
    TD* td,
    long func,
    long arg)
{
    Callsite* callsite = td->callsites;

    if (!callsite)
        return;

    td->oret_func = func;
    td->oret_arg = arg;

    OE_Longjmp(&callsite->jmpbuf, 1);
}

/*
**==============================================================================
**
** __OE_OCall()
**
**     Initiate a call into the host (exiting the enclave).
**
**==============================================================================
*/

OE_Result __OE_OCall(
    int func,
    oe_uint64_t argIn,
    oe_uint64_t* argOut)
{
    OE_Result result = OE_UNEXPECTED;
    TD* td = TD_Get();
    Callsite* callsite = td->callsites;

    /* Check for unexpected failures */
    if (!callsite)
        OE_TRY(OE_UNEXPECTED);

    /* Check for unexpected failures */
    if (!TD_Initialized(td))
        OE_TRY(OE_FAILURE);

    /* Save call site where execution will resume after OCALL */
    if (OE_Setjmp(&callsite->jmpbuf) == 0)
    {
        /* Exit, giving control back to the host so it can handle OCALL */
        _HandleExit(OE_CODE_OCALL, func, argIn);

        /* Unreachable! Host will transfer control back to OE_Main() */
        OE_Abort();
    }
    else
    {
        if (argOut)
            *argOut = td->oret_arg;

        /* ORET here */
    }

    result = OE_OK;

catch:
    return result;
}

/*
**==============================================================================
**
** OE_CallHost()
**
**==============================================================================
*/

OE_Result OE_CallHost(
    const char *func,
    void *argsIn)
{
    OE_Result result = OE_UNEXPECTED;
    OE_CallHostArgs* args = OE_NULL;

    /* Reject invalid parameters */
    if (!func || !argsIn)
        OE_THROW(OE_INVALID_PARAMETER);

    /* Initialize the arguments */
    {
        if (!(args = OE_HostMalloc(sizeof(OE_CallHostArgs))))
            OE_THROW(OE_OUT_OF_MEMORY);

        if (!(args->func = OE_HostStrdup(func)))
            OE_THROW(OE_OUT_OF_MEMORY);
    
        args->args = argsIn;
        args->result = OE_UNEXPECTED;
    }

    /* Call into the host */
    OE_TRY(__OE_OCall(OE_FUNC_CALL_HOST, (long)args, OE_NULL));

    /* Check the result */
    OE_TRY(args->result);

    result = OE_OK;

catch:

    if (args)
        OE_HostFree(args);

    return result;
}

/*
**==============================================================================
**
** __OE_HandleMain()
**
**     This function is called by OE_Main(), which is called by the EENTER 
**     instruction (executed by the host). The host passes the following 
**     parameters to EENTER:
**
**         RBX - TCS - address of a TCS page in the enclave
**         RCX - AEP - pointer to hosts's asynchronous exception procedure
**         RDI - ARGS1 (holds the CODE and FUNC parameters)
**         RSI - ARGS2 (holds the pointer to the args structure)
**
**     EENTER then calls OE_Main() with the following registers:
**
**         RAX - CSSA - index of current SSA
**         RBX - TCS - address of TCS
**         RCX - RETADDR - address to jump back to on EEXIT
**         RDI - ARGS1 (holds the code and func parameters)
**         RSI - ARGS2 (holds the pointer to the args structure)
**
**     Finally OE_Main() calls this function with the following parameters:
**
**         ARGS1 (holds the code and func parameters)
**         ARGS2 (holds the pointer to the args structure)
**         CSSA - index of current SSA
**         TCS - address of TCS (thread control structure)
**
**     Each enclave contains one or more thread sections (a collection of pages
**     used by a thread entering the enclave). Each thread section has the
**     following layout:
**
**         +--------------------------------+
**         | Guard Page                     |
**         +--------------------------------+
**         | Stack pages                    |
**         +--------------------------------+
**         | Guard Page                     |
**         +--------------------------------+
**         | TCS Page                       |
**         +--------------------------------+
**         | SSA (State Save Area) 0        |
**         +--------------------------------+
**         | SSA (State Save Area) 1        |
**         +--------------------------------+
**         | Guard Page                     |
**         +--------------------------------+
**         | GS page (contains thread data) |
**         +--------------------------------+
**
**     EENTER sets the GS segment register to refer to the GS page before
**     calling this function.
**
**     If the enclave should fault, SGX saves the registers in the SSA slot
**     (given by CSSA) and invokes the host's asynchronous exception handler
**     (AEP). The handler may terminate or call ERESUME which increments CSSA
**     and enters this function again. So:
**
**         CSSA == 0: indicates a normal entry
**         CSSA >= 1: indicates an exception entry
**
**     Since the enclave builder only allocates two SSA pages, the enclave can
**     nest no more than two faults. EENTER fails when the number of SSA slots
**     are exhausted (i.e., TCS.CSSA == TCS.NSSA)
**
**     This function ultimately calls EEXIT to exit the enclave. An enclave may
**     exit to the host for two reasons (asside from an asynchronous exception 
**     already mentioned):
**
**         (1) To return normally from an ECALL
**         (2) To initiate an OCALL
**
**     When exiting to perform an OCALL, the host may perform another ECALL,
**     and so ECALLS and OCALLS may be nested arbitrarily until stack space is
**     exhausted (hitting a guard page). The state for performing nested calls
**     is maintained on the stack associated with the TCS (see diagram above).
**
**     The enclave's stack pointer is determined as follows:
**
**         (*) For non-nested calls, the stack pointer is calculated relative
**             to the TCS (one page before minus the STATIC stack size).
**
**         (*) For nested calls the stack pointer is obtained from the 
**             TD.last_sp field (saved by the previous call).
**
**==============================================================================
*/

void __OE_HandleMain(
    oe_uint64_t arg1,
    oe_uint64_t arg2,
    oe_uint64_t cssa,
    void* tcs)
{
    OE_Code code = OE_HI_WORD(arg1);
    int func = OE_LO_WORD(arg1);
    oe_uint64_t argIn = arg2;

    /* Get pointer to the thread data structure */
    TD* td = TD_FromTCS(tcs);
    
    /* Initialize thread data structure (if not already initialized) */
    if (!TD_Initialized(td))
        TD_Init(td);

    /* Initialize the enclave the first time it is ever entered */
    if (td->initialized == 0)
        OE_InitializeEnclave(td);

    /* If this is a normal (non-exception) entry */
    if (cssa == 0)
    {
        if (code == OE_CODE_ECALL)
        {
            _HandleECALL(td, func, argIn);
            return;
        }
        else if (code == OE_CODE_ORET)
        {
            /* Eventually calls OE_Exit() and never returns here */
            _HandleORET(td, func, argIn);
        }
        else
        {
            /* Unexpected case */
            OE_Abort();
        }
    }
    else /* cssa > 0 */
    {
        /* ATTN: handle asynchronous exception (AEX) */
        OE_Abort();
    }
}
