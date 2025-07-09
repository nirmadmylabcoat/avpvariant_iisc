import subprocess
import time

MAX_M = 150  # Max number of parties to test

def run(cmd):
    """Run a shell command and return its stdout."""
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.stdout.strip()

def kill_avp_processes():
    """Clean up any hanging AVP-related processes."""
    subprocess.run([
        "powershell", "-Command",
        "Stop-Process -Name avpgenb, avpgeny, avpencode, avpmodtally -Force -ErrorAction SilentlyContinue"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def run_batch_executables(exe_name, args_per_party):
    """Launch multiple instances of the same executable with per-party arguments and wait for all."""
    for args in args_per_party:
        arglist = ' '.join(str(arg) for arg in args)
        ps_cmd = f'''
        $proc = Start-Process -NoNewWindow -FilePath .\\{exe_name} -ArgumentList "{arglist}" -PassThru;
        $proc.WaitForExit()
        '''
        subprocess.run(["powershell", "-Command", ps_cmd], check=True)

def test_m_parties(m):
    print(f"\n🔍 Testing with m = {m} parties...")

    kill_avp_processes()
    run("avpmodclean.exe")

    # Step 1: Init
    run("avpmodinit.exe")

    # Step 2: avpgenb.exe (b-generation)
    run_batch_executables("avpgenb.exe", [(i, m) for i in range(m)])
    time.sleep(5)

    # Step 3: avpgeny.exe (y-generation)
    run_batch_executables("avpgeny.exe", [(i, m) for i in range(m)])
    time.sleep(5)

    # Step 4: avpencode.exe (all vote 0)
    run_batch_executables("avpencode.exe", [(i, 0, m) for i in range(m)])
    time.sleep(20)

    # Step 5: tally
    result = subprocess.run(f"avpmodtally.exe {m}", shell=True, capture_output=True, text=True)
    output = result.stdout.strip()
    print(output)

    if "Final vote tally: 0" not in output:
        print(f"\n❌ Protocol failed at m = {m}. ✅ Max safe m = {m - 1}")
        return False
    return True

def main():
    for m in range(80, MAX_M + 1):
        if not test_m_parties(m):
            break
    else:
        print(f"\n✅ SUCCESS: Protocol passed for all m = {MAX_M} parties.")

if __name__ == "__main__":
    main()
